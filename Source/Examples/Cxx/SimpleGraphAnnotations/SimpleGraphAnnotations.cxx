#include "vtkActor.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLayers.h"
#include "vtkCircularLayoutStrategy.h"
#include "vtkGraphAnnotationLayersFilter.h"
#include "vtkGraphLayout.h"
#include "vtkGraphMapper.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInteractorStyleAnnotate2D.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"

/*
 * This example shows how vtkGraphAnnotationLayers filter is used to add
 * annotations or "landmarks" to a graph drawing.
 */

void CreateGraph(vtkMutableUndirectedGraph* graph)
{
  graph->SetNumberOfVertices(10);
  graph->AddEdge(0, 1);
  graph->AddEdge(0, 3);
  graph->AddEdge(0, 5);
  graph->AddEdge(1, 3);
  graph->AddEdge(3, 9);
  graph->AddEdge(5, 4);
  graph->AddEdge(5, 6);
  graph->AddEdge(5, 7);
  graph->AddEdge(5, 8);
  graph->AddEdge(5, 9);
}

void CreateAnnotation(vtkAnnotation* annotation)
{
  // Each selection node will be used to create one convex hull.
  vtkSmartPointer<vtkIdTypeArray> vertexIndices1 =
    vtkSmartPointer<vtkIdTypeArray>::New();
  vertexIndices1->InsertNextValue(2);

  vtkSmartPointer<vtkSelectionNode> selectionNode1 =
    vtkSmartPointer<vtkSelectionNode>::New();
  selectionNode1->SetFieldType(vtkSelectionNode::VERTEX);
  selectionNode1->SetContentType(vtkSelectionNode::INDICES);
  selectionNode1->SetSelectionList(vertexIndices1);

  vtkSmartPointer<vtkIdTypeArray> vertexIndices2 =
    vtkSmartPointer<vtkIdTypeArray>::New();
  vertexIndices2->InsertNextValue(5);
  vertexIndices2->InsertNextValue(6);
  vertexIndices2->InsertNextValue(7);
  vertexIndices2->InsertNextValue(8);
  vertexIndices2->InsertNextValue(9);

  vtkSmartPointer<vtkSelectionNode> selectionNode2 =
    vtkSmartPointer<vtkSelectionNode>::New();
  selectionNode2->SetFieldType(vtkSelectionNode::VERTEX);
  selectionNode2->SetContentType(vtkSelectionNode::INDICES);
  selectionNode2->SetSelectionList(vertexIndices2);

  // The selection nodes are added to a selection.
  vtkSmartPointer<vtkSelection> selection =
    vtkSmartPointer<vtkSelection>::New();
  selection->AddNode(selectionNode1);
  selection->AddNode(selectionNode2);

  // An annotation contains one selection.
  annotation->SetSelection(selection);

  // An annotion holds various properties that will be used to create the representation.
  annotation->GetInformation()->Set(vtkAnnotation::COLOR(), 1.0, 0.0, 0.0);
  annotation->GetInformation()->Set(vtkAnnotation::OPACITY(), 0.7);
  annotation->GetInformation()->Set(vtkAnnotation::LABEL(), "An Annotation");
  annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), true);
}

int main(int argc, char* argv[])
{
  // Create a graph and layout.
  vtkSmartPointer<vtkMutableUndirectedGraph> graph =
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
  vtkSmartPointer<vtkCircularLayoutStrategy> strategy =
    vtkSmartPointer<vtkCircularLayoutStrategy>::New();
  vtkSmartPointer<vtkGraphLayout> layout =
    vtkSmartPointer<vtkGraphLayout>::New();
  CreateGraph(graph);
  layout->SetInput(graph);
  layout->SetLayoutStrategy(strategy);

  // Create a visual representation of the graph.
  vtkSmartPointer<vtkGraphMapper> graphMapper =
    vtkSmartPointer<vtkGraphMapper>::New();
  vtkSmartPointer<vtkActor> graphActor =
    vtkSmartPointer<vtkActor>::New();
  graphMapper->SetInputConnection(layout->GetOutputPort());
  graphActor->SetMapper(graphMapper);


  // Create a vtkAnnotationLayers and add an annotation.
  vtkSmartPointer<vtkAnnotationLayers> annotationLayers =
    vtkSmartPointer<vtkAnnotationLayers>::New();
  vtkSmartPointer<vtkAnnotation> annotation =
    vtkSmartPointer<vtkAnnotation>::New();
  CreateAnnotation(annotation);
  annotationLayers->AddAnnotation(annotation);

  // Generate convex hulls around the selection nodes in the annotations.
  // Ensure the hulls cover at least 20 pixels. If a selection node has only
  // one vertex a rectangle is produced.
  vtkSmartPointer<vtkGraphAnnotationLayersFilter> annotationsFilter =
    vtkSmartPointer<vtkGraphAnnotationLayersFilter>::New();
  annotationsFilter->SetHullShapeToConvexHull();
  annotationsFilter->SetMinHullSizeInWorld(0.00001);
  annotationsFilter->SetMinHullSizeInDisplay(20);
  annotationsFilter->SetScaleFactor(1.2);
  annotationsFilter->SetInputConnection(0, layout->GetOutputPort());
  annotationsFilter->SetInput(1, annotationLayers);

  // Create a visual representation of the annotations.
  vtkSmartPointer<vtkPolyDataMapper> annotationsMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> annotationsActor =
    vtkSmartPointer<vtkActor>::New();
  annotationsMapper->SelectColorArray("Hull color");
  annotationsMapper->SetScalarModeToUseCellFieldData();
  annotationsMapper->SetScalarVisibility(true);
  annotationsMapper->SetInputConnection(annotationsFilter->GetOutputPort());
  annotationsActor->SetMapper(annotationsMapper);


  // Add the graph drawing and the annotations to a renderer. The annotations
  // filter needs a reference to the renderer to calculate the minimum hull size.
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  annotationsFilter->SetRenderer(renderer);
  renderer->AddActor(graphActor);
  renderer->AddActor(annotationsActor);

  // The annotations (and the graph) are drawn in the plane so  a 2D interactor
  // works best.
  vtkSmartPointer<vtkInteractorStyleAnnotate2D> style =
      vtkSmartPointer<vtkInteractorStyleAnnotate2D>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetInteractorStyle(style);
  vtkSmartPointer<vtkRenderWindow> window =
    vtkSmartPointer<vtkRenderWindow>::New();
  window->AddRenderer(renderer);
  window->SetInteractor(interactor);
  interactor->Initialize();
  interactor->Start();

  return EXIT_SUCCESS;
}
