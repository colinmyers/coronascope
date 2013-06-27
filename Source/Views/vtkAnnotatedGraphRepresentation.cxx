/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAnnotatedGraphRepresentation.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkAnnotatedGraphRepresentation.h"

#include "vtkActor.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLink.h"
#include "vtkApplyColors.h"
#include "vtkCellCenters.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkConvertSelection.h"
#include "vtkDataSetAttributes.h"
#include "vtkGraph.h"
#include "vtkGraphAnnotationLayersFilter.h"
#include "vtkGraphLayout.h"
#include "vtkGraphToGlyphs.h"
#include "vtkGraphToPolyData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolygon.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkStringArray.h"
#include "vtkTable.h"


vtkStandardNewMacro(vtkAnnotatedGraphRepresentation)

//----------------------------------------------------------------------------
vtkAnnotatedGraphRepresentation::vtkAnnotatedGraphRepresentation()
{
  this->LastSelectionNode = 0;

  this->LandmarkGlyph = vtkSmartPointer<vtkGraphAnnotationLayersFilter>::New();
  this->LandmarkMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LandmarkActor = vtkSmartPointer<vtkActor>::New();

  this->LandmarkOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LandmarkOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->LandmarkCentres = vtkSmartPointer<vtkCellCenters>::New();
  this->LandmarkLabelHierarchy =
      vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();

  this->LandmarkGlyph->SetInputConnection(this->ApplyColors->GetOutputPort());
  this->LandmarkMapper->SetInputConnection(
      this->LandmarkGlyph->GetOutputPort(0));
  this->LandmarkActor->SetMapper(this->LandmarkMapper);
  this->LandmarkOutlineMapper->SetInputConnection(
      this->LandmarkGlyph->GetOutputPort(1));
  this->LandmarkOutlineActor->SetMapper(this->LandmarkOutlineMapper);

  this->LandmarkCentres->SetInputConnection(
      this->LandmarkGlyph->GetOutputPort(0));
  this->LandmarkLabelHierarchy->SetInput(this->EmptyPolyData);

  this->OutlineGlyph->SetGlyphType(vtkGraphToGlyphs::CIRCLE);
  this->OutlineMapper->SetScalarModeToUseCellFieldData();
  this->OutlineMapper->SetScalarVisibility(true);
  this->OutlineActor->SetVisibility(false);

  this->VertexMapper->SetScalarModeToUseCellFieldData();
  this->VertexMapper->SetScalarVisibility(false);
  this->VertexMapper->SelectColorArray(0);

  this->EdgeActor->SetPosition(0, 0, -0.001);
  this->LandmarkGlyph->SetMinHullSizeInDisplay(0);
  this->LandmarkGlyph->SetMinHullSizeInWorld(0.0001);

  this->LandmarkMapper->SelectColorArray("Hull color");
  this->LandmarkMapper->SetScalarModeToUseCellFieldData();
  this->LandmarkMapper->SetScalarVisibility(true);
  this->LandmarkActor->SetPickable(false);
  this->LandmarkActor->GetProperty()->BackfaceCullingOff();
  this->LandmarkActor->SetPosition(0.0, 0.0, 0.001);
  this->LandmarkActor->GetProperty()->SetOpacity(0.999);

  this->LandmarkOutlineMapper->SelectColorArray("Hull color");
  this->LandmarkOutlineMapper->SetScalarModeToUseCellFieldData();
  this->LandmarkOutlineMapper->SetScalarVisibility(true);
  this->LandmarkOutlineActor->SetPickable(false);
  this->LandmarkOutlineActor->GetProperty()->SetLineWidth(3.0);
  this->LandmarkOutlineActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->LandmarkOutlineActor->GetProperty()->SetOpacity(0.999);
  this->LandmarkOutlineActor->SetPosition(0.0, 0.0, 0.002);
}

//----------------------------------------------------------------------------
vtkAnnotatedGraphRepresentation::~vtkAnnotatedGraphRepresentation()
{
  //
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetDefaultVertexColor(double r, double g,
    double b, double a)
{
  this->ApplyColors->SetDefaultPointColor(r, g, b);
  this->ApplyColors->SetDefaultPointOpacity(a);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::GetDefaultVertexColor(double* rgba)
{
  this->ApplyColors->GetDefaultPointColor(rgba[0], rgba[1], rgba[2]);
  rgba[3] = this->ApplyColors->GetDefaultPointOpacity();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetDefaultEdgeColor(double r, double g,
    double b, double a)
{
  this->ApplyColors->SetDefaultCellColor(r, g, b);
  this->ApplyColors->SetDefaultCellOpacity(a);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::GetDefaultEdgeColor(double* rgba)
{
  this->ApplyColors->GetDefaultCellColor(rgba[0], rgba[1], rgba[2]);
  rgba[3] = this->ApplyColors->GetDefaultCellOpacity();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetSelectedVertexColour(double r,
    double g, double b, double a)
{
  this->ApplyColors->SetSelectedPointColor(r, g, b);
  this->ApplyColors->SetSelectedPointOpacity(a);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::GetSelectedVertexColour(double* rgba)
{
  this->ApplyColors->GetSelectedPointColor(rgba[0], rgba[1], rgba[2]);
  rgba[3] = this->ApplyColors->GetSelectedPointOpacity();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetSelectedEdgeColour(double r, double g,
    double b, double a)
{
  this->ApplyColors->SetSelectedCellColor(r, g, b);
  this->ApplyColors->SetSelectedCellOpacity(a);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::GetSelectedEdgeColour(double* rgba)
{
  this->ApplyColors->GetSelectedCellColor(rgba[0], rgba[1], rgba[2]);
  rgba[3] = this->ApplyColors->GetSelectedCellOpacity();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetUseCurrentAnnotationColour(bool b)
{
  this->ApplyColors->SetUseCurrentAnnotationColor(b);
}

//----------------------------------------------------------------------------
bool vtkAnnotatedGraphRepresentation::GetUseCurrentAnnotationColour()
{
  return this->ApplyColors->GetUseCurrentAnnotationColor();
}

//----------------------------------------------------------------------------
vtkStringArray* vtkAnnotatedGraphRepresentation::GetEdgeArrayNames()
{
  vtkGraph* g = vtkGraph::SafeDownCast(this->GetInput());
  int numArrays = g->GetEdgeData()->GetNumberOfArrays();
  vtkStringArray* names = vtkStringArray::New();
  names->SetNumberOfValues(numArrays);
  for (int i = 0; i < numArrays; ++i)
  {
    names->SetValue(i, g->GetEdgeData()->GetArrayName(i));
  }
  return names;
}

//----------------------------------------------------------------------------
vtkStringArray* vtkAnnotatedGraphRepresentation::GetVertexArrayNames()
{
  vtkGraph* g = vtkGraph::SafeDownCast(this->GetInput());
  int numArrays = g->GetVertexData()->GetNumberOfArrays();
  vtkStringArray* names = vtkStringArray::New();
  names->SetNumberOfValues(numArrays);
  for (int i = 0; i < numArrays; ++i)
  {
    names->SetValue(i, g->GetVertexData()->GetArrayName(i));
  }
  return names;
}

//----------------------------------------------------------------------------
// CSM_TODO For reasons which I cannot fathom, we must set the ApplyColors filter as
// modified to get enough of the pipeline to re-execute so that the newly scaled
// landmarks are rendered immediately.
void vtkAnnotatedGraphRepresentation::SetScaleFactor(double scale)
{
  this->LandmarkGlyph->SetScaleFactor(scale);
  this->ApplyColors->Modified();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetScaleToCamera(bool b)
{
  if (b)
  {
    this->LandmarkGlyph->SetMinHullSizeInDisplay(35);
    this->ApplyColors->Modified();
  }
  else
  {
    this->LandmarkGlyph->SetMinHullSizeInDisplay(0);
    this->ApplyColors->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetMinimumSize(unsigned size)
{
  this->LandmarkGlyph->SetMinHullSizeInDisplay(size);
  this->ApplyColors->Modified();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetOutlineVisibility(bool b)
{
  this->LandmarkGlyph->SetOutline(b);
  this->ApplyColors->Modified();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkOutlineColour(double r,
    double g, double b, double a)
{
  this->LandmarkOutlineActor->GetProperty()->SetColor(r, g, b);
  this->LandmarkOutlineActor->GetProperty()->SetOpacity(a);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::GetLandmarkOutlineColour(double* rgba)
{
  this->LandmarkOutlineActor->GetProperty()->GetColor(rgba[0], rgba[1],
      rgba[2]);
  rgba[3] = this->LandmarkOutlineActor->GetProperty()->GetOpacity();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkToConvexHull()
{
  this->LandmarkGlyph->SetHullShapeToConvexHull();
  this->ApplyColors->Modified();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkToRectangle()
{
  this->LandmarkGlyph->SetHullShapeToBoundingRectangle();
  this->ApplyColors->Modified();
}

//----------------------------------------------------------------------------
vtkCellCenters* vtkAnnotatedGraphRepresentation::GetLandmarkCentres()
{
  return this->LandmarkCentres;
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkLabelArrayName(
    const char* name)
{
  this->LandmarkLabelHierarchy->SetLabelArrayName(name);
}

//----------------------------------------------------------------------------
const char* vtkAnnotatedGraphRepresentation::GetLandmarkLabelArrayName()
{
  return this->LandmarkLabelHierarchy->GetLabelArrayName();
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkLabelVisibility(bool b)
{
  if (b)
  {
    this->LandmarkLabelHierarchy->SetInputConnection(
        this->LandmarkCentres->GetOutputPort());
  }
  else
  {
    this->LandmarkLabelHierarchy->SetInput(this->EmptyPolyData);
  }
}

//----------------------------------------------------------------------------
bool vtkAnnotatedGraphRepresentation::GetLandmarkLabelVisibility()
{
  return !(this->LandmarkLabelHierarchy->GetInput()
      == (vtkDataObject*) this->EmptyPolyData);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::SetLandmarkLabelTextProperty(
    vtkTextProperty* p)
{
  this->LandmarkLabelHierarchy->SetTextProperty(p);
}

//----------------------------------------------------------------------------
vtkTextProperty* vtkAnnotatedGraphRepresentation::GetLandmarkLabelTextProperty()
{
  return this->LandmarkLabelHierarchy->GetTextProperty();
}

//----------------------------------------------------------------------------
bool vtkAnnotatedGraphRepresentation::AddToView(vtkView* view)
{
  this->Superclass::AddToView(view);
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (rv)
  {
    this->LandmarkGlyph->SetRenderer(rv->GetRenderer());
    rv->GetRenderer()->AddActor(this->LandmarkActor);
    rv->GetRenderer()->AddActor(this->LandmarkOutlineActor);
    rv->AddLabels(this->LandmarkLabelHierarchy->GetOutputPort());
    rv->RegisterProgress(this->LandmarkGlyph);
    rv->RegisterProgress(this->LandmarkCentres);
    rv->RegisterProgress(this->LandmarkMapper);
    rv->RegisterProgress(this->LandmarkOutlineMapper);
    rv->RegisterProgress(this->LandmarkLabelHierarchy);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkAnnotatedGraphRepresentation::RemoveFromView(vtkView* view)
{
  this->Superclass::RemoveFromView(view);
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (rv)
  {
    this->LandmarkGlyph->SetRenderer(0);
    rv->GetRenderer()->RemoveActor(this->LandmarkActor);
    rv->GetRenderer()->RemoveActor(this->LandmarkOutlineActor);
    rv->RemoveLabels(this->LandmarkLabelHierarchy->GetOutputPort());
    rv->UnRegisterProgress(this->LandmarkGlyph);
    rv->UnRegisterProgress(this->LandmarkCentres);
    rv->UnRegisterProgress(this->LandmarkMapper);
    rv->UnRegisterProgress(this->LandmarkOutlineMapper);
    rv->UnRegisterProgress(this->LandmarkLabelHierarchy);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
int vtkAnnotatedGraphRepresentation::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  if (!this->Superclass::RequestData(request, inputVector, outputVector))
  {
    return 0;
  }
  this->LandmarkGlyph->SetInputConnection(1,
      this->GetInternalAnnotationOutputPort());
  return 1;
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::LassoSelect(vtkPoints* lassoPoints,
    bool annotate)
{
  if (lassoPoints == 0)
  {
    return;
  }
  if (lassoPoints->GetNumberOfPoints() == 0)
  {
    return;
  }
  lassoPoints->ComputeBounds();
  double* bounds = lassoPoints->GetBounds();
  int numSelectionPoints = lassoPoints->GetNumberOfPoints();

  vtkGraph* graph = vtkGraph::SafeDownCast(this->Layout->GetOutput());
  vtkPoints* graphPoints = graph->GetPoints();
  vtkIdType numGraphPoints = graphPoints->GetNumberOfPoints();
  if (numGraphPoints == 0)
  {
    return;
  }

  double graphPoint[3];
  double n[3];
  vtkPolygon::ComputeNormal(graphPoints, n);
  vtkSmartPointer<vtkIdTypeArray> outPointIds =
      vtkSmartPointer<vtkIdTypeArray>::New();
  for (vtkIdType i = 0; i < numGraphPoints; ++i)
  {
    graphPoints->GetPoint(i, graphPoint);
    graphPoint[2] = 0.0;
    int result = vtkPolygon::PointInPolygon(graphPoint, numSelectionPoints,
        static_cast<double*>(lassoPoints->GetData()->GetVoidPointer(0)),
        bounds, n);
    if (result == -1)
    {
      vtkWarningMacro(<< "Lasso select: Degenerate polygon");
      return;
    }
    else if (result == 1)
    {
      outPointIds->InsertNextValue(i);
    }
  }
  if (outPointIds->GetNumberOfTuples() > 0)
  {
    vtkSmartPointer<vtkSelection> selection =
        vtkSmartPointer<vtkSelection>::New();
    vtkSmartPointer<vtkSelectionNode> node =
        vtkSmartPointer<vtkSelectionNode>::New();
    node->SetContentType(vtkSelectionNode::INDICES);
    node->SetFieldType(vtkSelectionNode::VERTEX);
    node->SetSelectionList(outPointIds);
    selection->AddNode(node);
    this->LastSelectionNode = node;

    if (annotate)
    {
      vtkSmartPointer<vtkAnnotationLayers> annotationLayers = vtkSmartPointer<
          vtkAnnotationLayers>::New();
      vtkSmartPointer<vtkAnnotation> annotation =
          vtkSmartPointer<vtkAnnotation>::New();
      annotation->SetSelection(selection);
      annotationLayers->AddAnnotation(annotation);
      this->UpdateAnnotations(annotationLayers, true);

    }
    else
    {
      this->UpdateSelection(selection, false);
    }
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::UndoLastAnnotation()
{
  vtkAnnotationLayers* annotationLayers =
      this->AnnotationLinkInternal->GetAnnotationLayers();
  unsigned int numAnnotationLayers = annotationLayers->GetNumberOfAnnotations();
  for (unsigned int i = 0; i < numAnnotationLayers; ++i)
  {
    annotationLayers->GetAnnotation(i)->GetSelection()->RemoveNode(
        this->LastSelectionNode);
  }
  this->InvokeEvent(vtkCommand::AnnotationChangedEvent,
      reinterpret_cast<void*>(annotationLayers));
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::Annotate(vtkView* view,
    vtkAnnotationLayers* annotations, bool extend)
{
  vtkAnnotationLayers* converted = this->ConvertAnnotations(view, annotations);
  if (converted)
  {
    this->UpdateAnnotations(converted, extend);
    if (converted != annotations)
    {
      converted->Delete();
    }
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::Select(vtkView* view,
    vtkSelection* selection, bool extend)
{
  if (this->Selectable)
  {
    vtkSelection* converted = this->ConvertSelection(view, selection);
    if (converted)
    {
      this->UpdateSelection(converted, extend);
      if (converted != selection)
      {
        converted->Delete();
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::UpdateAnnotations(
    vtkAnnotationLayers* annotations, bool extend)
{
  vtkAnnotationLayers* annotationLinkLayers =
      this->AnnotationLinkInternal->GetAnnotationLayers();
  vtkAnnotation* currentAnnotation =
      annotationLinkLayers->GetCurrentAnnotation();
  if (!currentAnnotation)
  {
    return;
  }

  if (extend && annotationLinkLayers->GetNumberOfAnnotations() > 0)
  {
    if (!currentAnnotation->GetSelection())
    {
      return;
    }
    if (currentAnnotation->GetSelection()->GetNumberOfNodes() == 0)
    {
      return;
    }
    if (!currentAnnotation->GetSelection()->GetNode(0)->GetSelectionList())
    {
      return;
    }
    // Remove empty node (no points)
    if (currentAnnotation->GetSelection()->GetNode(0)->GetSelectionList()->GetNumberOfTuples()
        == 0)
    {
      currentAnnotation->GetSelection()->RemoveNode(static_cast<unsigned>(0));
    }
    // Add the selection nodes to the current annotation
    for (unsigned i = 0; i < annotations->GetNumberOfAnnotations(); ++i)
    {
      vtkSelection* selection = annotations->GetAnnotation(i)->GetSelection();
      for (unsigned j = 0; j < selection->GetNumberOfNodes(); ++j)
      {
        currentAnnotation->GetSelection()->AddNode(selection->GetNode(j));
      }
    }
    this->InvokeEvent(vtkCommand::AnnotationChangedEvent,
        reinterpret_cast<void*>(annotationLinkLayers));
  }
  else
  {
    // Add a new annotation
    for (unsigned i = 0; i < annotations->GetNumberOfAnnotations(); ++i)
    {
      annotationLinkLayers->AddAnnotation(annotations->GetAnnotation(i));
    }
    annotationLinkLayers->SetCurrentAnnotation(
        annotations->GetCurrentAnnotation());
    this->InvokeEvent(vtkCommand::AnnotationChangedEvent,
        reinterpret_cast<void*>(annotationLinkLayers));
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::UpdateSelection(vtkSelection* selection,
    bool extend)
{
  if (!selection)
  {
    return;
  }
  if (selection->GetNumberOfNodes() == 0)
  {
    return;
  }
  if (extend)
  {
    selection->Union(this->AnnotationLinkInternal->GetCurrentSelection());
  }
  this->AnnotationLinkInternal->SetCurrentSelection(selection);
  this->InvokeEvent(vtkCommand::SelectionChangedEvent,
      reinterpret_cast<void*>(selection));
}

//----------------------------------------------------------------------------
vtkAnnotationLayers* vtkAnnotatedGraphRepresentation::ConvertAnnotations(
    vtkView* vtkNotUsed(view), vtkAnnotationLayers* annotations)
{
  vtkSelection* selection = this->ConvertSelection(0,
      annotations->GetCurrentAnnotation()->GetSelection());

  annotations->GetCurrentAnnotation()->SetSelection(selection);

  return annotations;
}

//----------------------------------------------------------------------------
vtkSelection* vtkAnnotatedGraphRepresentation::ConvertSelection(
    vtkView* vtkNotUsed(view), vtkSelection* sel)
{
  // Search for selection nodes relating to the vertex and edges
  // of the graph.
  vtkSmartPointer<vtkSelectionNode> vertexNode = vtkSmartPointer<
      vtkSelectionNode>::New();
  vtkSmartPointer<vtkSelectionNode> edgeNode =
      vtkSmartPointer<vtkSelectionNode>::New();
  bool foundEdgeNode = false;

  if (sel->GetNumberOfNodes() > 0)
  {
    for (unsigned int i = 0; i < sel->GetNumberOfNodes(); ++i)
    {
      vtkSelectionNode* node = sel->GetNode(i);
      vtkProp* prop = vtkProp::SafeDownCast(
          node->GetProperties()->Get(vtkSelectionNode::PROP()));
      if (node->GetContentType() == vtkSelectionNode::FRUSTUM)
      {
        // A frustum selection can be used to select vertices and edges.
        vertexNode->ShallowCopy(node);
        edgeNode->ShallowCopy(node);
        foundEdgeNode = true;
      }
      else if (prop == this->VertexActor.GetPointer())
      {
        // The prop on the selection matches the vertex actor, so
        // this must have been a visible cell selection.
        vertexNode->ShallowCopy(node);
      }
      else if (prop == this->EdgeActor.GetPointer())
      {
        // The prop on the selection matches the edge actor, so
        // this must have been a visible cell selection.
        edgeNode->ShallowCopy(node);
        //        foundEdgeNode = true;
        foundEdgeNode = false;
      }
    }
  }

  // Remove the prop to avoid reference loops.
  vertexNode->GetProperties()->Remove(vtkSelectionNode::PROP());
  edgeNode->GetProperties()->Remove(vtkSelectionNode::PROP());

  vtkSelection* converted = vtkSelection::New();
  vtkGraph* input = vtkGraph::SafeDownCast(this->GetInput());
  if (!input)
  {
    return converted;
  }

  bool selectedVerticesFound = false;
  if (vertexNode)
  {
    // Convert a cell selection on the glyphed vertices into a
    // vertex selection on the graph of the appropriate type.

    // First, convert the cell selection on the polydata to
    // a pedigree ID selection (or index selection if there are no
    // pedigree IDs).
    vtkSmartPointer<vtkSelection> vertexSel =
        vtkSmartPointer<vtkSelection>::New();
    vertexSel->AddNode(vertexNode);

    vtkPolyData* poly = vtkPolyData::SafeDownCast(
        this->VertexGlyph->GetOutput());
    vtkSmartPointer<vtkTable> temp = vtkSmartPointer<vtkTable>::New();
    temp->SetRowData(vtkPolyData::SafeDownCast(poly)->GetCellData());
    vtkSelection* polyConverted = 0;
    if (poly->GetCellData()->GetPedigreeIds())
    {
      polyConverted = vtkConvertSelection::ToSelectionType(vertexSel, poly,
          vtkSelectionNode::PEDIGREEIDS);
    }
    else
    {
      polyConverted = vtkConvertSelection::ToSelectionType(vertexSel, poly,
          vtkSelectionNode::INDICES);
    }

    // Now that we have a pedigree or index selection, interpret this
    // as a vertex selection on the graph, and convert it to the
    // appropriate selection type for this representation.
    for (unsigned int i = 0; i < polyConverted->GetNumberOfNodes(); ++i)
    {
      polyConverted->GetNode(i)->SetFieldType(vtkSelectionNode::VERTEX);
    }
    vtkSelection* vertexConverted = vtkConvertSelection::ToSelectionType(
        polyConverted, input, this->SelectionType, this->SelectionArrayNames);

    // For all output selection nodes, select all the edges among selected vertices.
    for (unsigned int i = 0; i < vertexConverted->GetNumberOfNodes(); ++i)
    {
      if ((vertexConverted->GetNode(i)->GetSelectionList()->GetNumberOfTuples()
          > 0) && (input->GetNumberOfEdges()) > 0)
      {
        // Get the list of selected vertices.
        selectedVerticesFound = true;
        vtkSmartPointer<vtkIdTypeArray> selectedVerts = vtkSmartPointer<
            vtkIdTypeArray>::New();
        vtkConvertSelection::GetSelectedVertices(vertexConverted, input,
            selectedVerts);

        // Get the list of induced edges on these vertices.
        vtkSmartPointer<vtkIdTypeArray> selectedEdges = vtkSmartPointer<
            vtkIdTypeArray>::New();
        input->GetInducedEdges(selectedVerts, selectedEdges);

        // Create an edge index selection containing the induced edges.
        vtkSmartPointer<vtkSelection> edgeSelection = vtkSmartPointer<
            vtkSelection>::New();
        vtkSmartPointer<vtkSelectionNode> edgeSelectionNode = vtkSmartPointer<
            vtkSelectionNode>::New();
        edgeSelectionNode->SetSelectionList(selectedEdges);
        edgeSelectionNode->SetContentType(vtkSelectionNode::INDICES);
        edgeSelectionNode->SetFieldType(vtkSelectionNode::EDGE);
        edgeSelection->AddNode(edgeSelectionNode);

        // Convert the edge selection to the appropriate type for this representation.
        vtkSelection* edgeConverted = vtkConvertSelection::ToSelectionType(
            edgeSelection, input, this->SelectionType,
            this->SelectionArrayNames);

        // Add the converted induced edge selection to the output selection.
        if (edgeConverted->GetNumberOfNodes() > 0)
        {
          converted->AddNode(edgeConverted->GetNode(0));
        }
        edgeConverted->Delete();
      }

      // Add the vertex selection node to the output selection.
      converted->AddNode(vertexConverted->GetNode(i));
    }
    polyConverted->Delete();
    vertexConverted->Delete();
  }
  if (foundEdgeNode && !selectedVerticesFound)
  {
    // If no vertices were found (hence no induced edges), look for
    // edges that were within the selection box.

    // First, convert the cell selection on the polydata to
    // a pedigree ID selection (or index selection if there are no
    // pedigree IDs).
    vtkSmartPointer<vtkSelection> edgeSel =
        vtkSmartPointer<vtkSelection>::New();
    edgeSel->AddNode(edgeNode);
    vtkPolyData* poly = vtkPolyData::SafeDownCast(
        this->GraphToPoly->GetOutput());
    vtkSelection* polyConverted = 0;
    if (poly->GetCellData()->GetPedigreeIds())
    {
      polyConverted = vtkConvertSelection::ToSelectionType(edgeSel, poly,
          vtkSelectionNode::PEDIGREEIDS);
    }
    else
    {
      polyConverted = vtkConvertSelection::ToSelectionType(edgeSel, poly,
          vtkSelectionNode::INDICES);
    }

    // Now that we have a pedigree or index selection, interpret this
    // as an edge selection on the graph, and convert it to the
    // appropriate selection type for this representation.
    for (unsigned int i = 0; i < polyConverted->GetNumberOfNodes(); ++i)
    {
      polyConverted->GetNode(i)->SetFieldType(vtkSelectionNode::EDGE);
    }

    // Convert the edge selection to the appropriate type for this representation.
    vtkSelection* edgeConverted = vtkConvertSelection::ToSelectionType(
        polyConverted, input, this->SelectionType, this->SelectionArrayNames);

    // Add the vertex selection node to the output selection.
    for (unsigned int i = 0; i < edgeConverted->GetNumberOfNodes(); ++i)
    {
      converted->AddNode(edgeConverted->GetNode(i));
    }
    polyConverted->Delete();
    edgeConverted->Delete();
  }
  return converted;
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
