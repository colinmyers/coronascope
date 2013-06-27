/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: vtkFlightMapFilter.cxx,v $

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkFlightMapFilter.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLayers.h"
#include "vtkBoostBrandesCentrality.h"
#include "vtkBoostConnectedComponents.h"
#include "vtkBoostKruskalMinimumSpanningTree.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkDirectedAcyclicGraph.h"
#include "vtkEdgeListIterator.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkOutEdgeIterator.h"
#include "vtkPoints.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkVariantArray.h"

#include <map>
#include <queue>
#include <vector>


vtkStandardNewMacro(vtkFlightMapFilter)

//-----------------------------------------------------------------------------
vtkFlightMapFilter::vtkFlightMapFilter()
{
  this->SetNumberOfInputPorts(2);

  this->Kruskal = vtkSmartPointer<vtkBoostKruskalMinimumSpanningTree>::New();
  this->MinimumSpanningTree = true;
  this->Kruskal->SetNegateEdgeWeights(false);

  this->ExtractKruskalSelection =
      vtkSmartPointer<vtkExtractSelectedGraph>::New();
  this->ExtractKruskalSelection->SetRemoveIsolatedVertices(false);
  this->ExtractKruskalSelection->SetSelectionConnection(
      this->Kruskal->GetOutputPort());

  this->EdgeLengthFactor = 1.0;
  this->EdgeDegreeFactor = 1.0;
  this->EdgeLandmarkProximityFactor = 1.0;
}

//-----------------------------------------------------------------------------
vtkFlightMapFilter::~vtkFlightMapFilter()
{
  //
}

//-----------------------------------------------------------------------------
int vtkFlightMapFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUndirectedGraph");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkAnnotationLayers");
    return 1;
  }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkFlightMapFilter::FillOutputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkUndirectedGraph");
  }
  else
  {
    return 0;
  }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkFlightMapFilter::SetExpressPreset()
{
  this->SetEdgeLengthFactor(1.0);
  this->SetEdgeDegreeFactor(0.5);
  this->SetEdgeLandmarkProximityFactor(0.0);
  this->SetMinimumSpanningTree(true);
}

//-----------------------------------------------------------------------------
void vtkFlightMapFilter::SetTouristPreset()
{
  this->SetEdgeLengthFactor(0.0);
  this->SetEdgeDegreeFactor(1.0);
  this->SetEdgeLandmarkProximityFactor(1.0);
  this->SetMinimumSpanningTree(false);
}

//-----------------------------------------------------------------------------
int vtkFlightMapFilter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo0 = outputVector->GetInformationObject(0);

  // Get the input and output data
  vtkGraph *inputGraph = vtkGraph::SafeDownCast(
      inInfo0->Get(vtkDataObject::DATA_OBJECT()));

  vtkAnnotationLayers *inputAnnotation = vtkAnnotationLayers::SafeDownCast(
      inInfo1->Get(vtkDataObject::DATA_OBJECT()));

  vtkGraph *outputGraph = vtkGraph::SafeDownCast(
      outInfo0->Get(vtkDataObject::DATA_OBJECT()));

  // Copy input graph into a mutable
  vtkSmartPointer<vtkMutableUndirectedGraph> mutableGraph = vtkSmartPointer<
      vtkMutableUndirectedGraph>::New();
  mutableGraph->DeepCopy(inputGraph);

  // Flatten the points into 2D
  double p[3];
  vtkPoints* inputPoints = mutableGraph->GetPoints();
  for (vtkIdType i = 0; i < inputPoints->GetNumberOfPoints(); ++i)
  {
    inputPoints->GetPoint(i, p);
    p[2] = 0.0;
    inputPoints->SetPoint(i, p);
  }

  //// Initialize and add default edge weights.
  vtkIdType numEdges = mutableGraph->GetNumberOfEdges();
  vtkDoubleArray* weights = vtkDoubleArray::New();
  weights->SetName("weights");
  weights->SetNumberOfValues(numEdges);
  for (vtkIdType i = 0; i < numEdges; ++i)
  {
    weights->SetValue(i, vtkFlightMapFilter::DefaultWeight);
  }

  //// Weight by edge length.
  vtkEdgeListIterator* edges = vtkEdgeListIterator::New();
  if (this->EdgeLengthFactor > 0.0)
  {
    mutableGraph->GetEdges(edges);
    while (edges->HasNext())
    {
      vtkEdgeType edge = edges->Next();
      double weight = sqrt(
          (inputPoints->GetPoint(edge.Source)[0] -
          inputPoints->GetPoint(edge.Target)[0]) *
          (inputPoints->GetPoint(edge.Source)[0] -
          inputPoints->GetPoint(edge.Target)[0]) +
          (inputPoints->GetPoint(edge.Source)[1] -
          inputPoints->GetPoint(edge.Target)[1]) *
          (inputPoints->GetPoint(edge.Source)[1] -
          inputPoints->GetPoint(edge.Target)[1]));
      weights->SetValue(edge.Id,
          weights->GetValue(edge.Id)
          - (weight * this->EdgeLengthFactor * 10.0));
    }
  }

  //// Weight by degree.
  if (this->EdgeDegreeFactor > 0.0)
  {
    mutableGraph->GetEdges(edges);
    while (edges->HasNext())
    {
      vtkEdgeType edge = edges->Next();
      double weight = mutableGraph->GetDegree(edge.Source)
              + mutableGraph->GetDegree(edge.Target);
      weights->SetValue(edge.Id,
          weights->GetValue(edge.Id)
          - (weight * this->EdgeDegreeFactor * 10.0));
    }
  }

  //// Weight by proximity to landmarks.
  if (this->EdgeLandmarkProximityFactor > 0.0)
  {
    // Gather up all annotation ids.
    vtkIdTypeArray* ids = vtkIdTypeArray::New();
    unsigned int numAnnotations = inputAnnotation->GetNumberOfAnnotations();
    for (unsigned int i = 0; i < numAnnotations; ++i)
    {
      vtkAnnotation* annotation = inputAnnotation->GetAnnotation(i);
      if (annotation->GetInformation()->Get(vtkAnnotation::ENABLE()))
      {
        vtkSelection* selection =
            inputAnnotation->GetAnnotation(i)->GetSelection();
        unsigned int numNodes = selection->GetNumberOfNodes();
        for (unsigned int j = 0; j < numNodes; ++j)
        {
          vtkSelectionNode* node = selection->GetNode(j);
          vtkIdTypeArray* arr = vtkIdTypeArray::SafeDownCast(
              node->GetSelectionList());
          vtkIdType numValues = arr->GetNumberOfTuples();
          for (vtkIdType k = 0; k < numValues; ++k)
          {
            ids->InsertNextValue(arr->GetValue(k));
          }
        }
      }
    }

    mutableGraph->GetEdges(edges);
    while (edges->HasNext())
    {
      vtkEdgeType edge = edges->Next();
      double weight = (ids->LookupValue(edge.Source) >= 0)
              + (ids->LookupValue(edge.Source) >= 0);
      weights->SetValue(edge.Id,
          weights->GetValue(edge.Id)
          - (weight * this->EdgeLandmarkProximityFactor * 50.0));
    }
    ids->Delete();
  }
  mutableGraph->GetEdgeData()->AddArray(weights);
  edges->Delete();
  weights->Delete();

  //// Find MST
  this->Kruskal->SetNegateEdgeWeights(!this->MinimumSpanningTree);
  this->Kruskal->SetInput(mutableGraph);
  this->Kruskal->SetEdgeWeightArrayName("weights");
  this->ExtractKruskalSelection->SetInput(mutableGraph);
  this->ExtractKruskalSelection->Update();

  // Copy results to output
  vtkGraph* mstGraph = this->ExtractKruskalSelection->GetOutput();
  if (!outputGraph->CheckedShallowCopy(mstGraph))
  {
    vtkErrorMacro(
        << "Error creating flight map: invalid output graph structure.");
    return 0;
  }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkFlightMapFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MinimumSpanningTree: " <<
          (this->MinimumSpanningTree ? "On" : "Off") << endl;
  os << indent << "EdgeLengthFactor: " << this->EdgeLengthFactor << endl;
  os << indent << "EdgeDegreeFactor: " << this->EdgeDegreeFactor << endl;
  os << indent << "EdgeLandmarkProximityFactor: " <<
      this->EdgeLandmarkProximityFactor << endl;
}
