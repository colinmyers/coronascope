/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAnnotatedGraphView.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkAnnotatedGraphView.h"

#include "vtkAnnotatedGraphRepresentation.h"
#include "vtkAnnotation.h"
#include "vtkAnnotationLayers.h"
#include "vtkCommand.h"
#include "vtkCoordinate.h"
#include "vtkDirectedGraph.h"
#include "vtkDoubleArray.h"
#include "vtkHardwareSelector.h"
#include "vtkInformation.h"
#include "vtkInteractorStyleAnnotate2D.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedIntArray.h"


vtkStandardNewMacro(vtkAnnotatedGraphView)

//----------------------------------------------------------------------------
vtkAnnotatedGraphView::vtkAnnotatedGraphView()
{
  this->SetInteractionModeTo2D();
  this->SetSelectionModeToFrustum();
  this->SetEdgeSelection(false);
  vtkInteractorStyleAnnotate2D* style = vtkInteractorStyleAnnotate2D::New();
  style->AddObserver(vtkCommand::SelectionChangedEvent, this->GetObserver());
  style->AddObserver(vtkCommand::AnnotationChangedEvent, this->GetObserver());
  style->AddObserver(vtkCommand::UserEvent, this->GetObserver());
  this->SetInteractorStyle(style);
  style->Delete();
  this->ReuseSingleRepresentationOn();

  // CSM_TODO Set multisamples to zero, otherwise we get garbage in the pixel
  // buffer when using the rubber band. This seems to be a problem with the
  // events/render sequencing between this class and the superclass.
  this->RenderWindow->SetMultiSamples(0);
}

//----------------------------------------------------------------------------
vtkAnnotatedGraphView::~vtkAnnotatedGraphView()
{
  //
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphView::SetAnnotatedGraphRepresentation(
    vtkAnnotatedGraphRepresentation* repr)
{
  this->SetRepresentation(repr);
}

//----------------------------------------------------------------------------
vtkAnnotatedGraphRepresentation* vtkAnnotatedGraphView::GetAnnotatedGraphRepresentation()
{
  vtkAnnotatedGraphRepresentation* graphRep = 0;
  for (int i = 0; i < this->GetNumberOfRepresentations(); ++i)
  {
    vtkDataRepresentation* rep = this->GetRepresentation(i);
    graphRep = vtkAnnotatedGraphRepresentation::SafeDownCast(rep);
    if (graphRep)
    {
      break;
    }
  }
  if (!graphRep)
  {
    vtkSmartPointer<vtkDirectedGraph> g =
        vtkSmartPointer<vtkDirectedGraph>::New();
    graphRep = vtkAnnotatedGraphRepresentation::SafeDownCast(
        this->AddRepresentationFromInput(g));
  }
  return graphRep;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkAnnotatedGraphView::CreateDefaultRepresentation(
    vtkAlgorithmOutput* port)
{
  vtkAnnotatedGraphRepresentation* rep = vtkAnnotatedGraphRepresentation::New();
  rep->SetInputConnection(port);
  return rep;
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphView::ProcessEvents(vtkObject* caller,
    unsigned long eventId, void* callData)
{
  if (caller == this->GetInteractorStyle())
  {
    if (eventId == vtkCommand::UserEvent)
    {
      this->GetAnnotatedGraphRepresentation()->UndoLastAnnotation();
      this->Render();
    }

    else if (eventId == vtkCommand::AnnotationChangedEvent
        || eventId == vtkCommand::SelectionChangedEvent)
    {
      vtkUnsignedIntArray* displayPoints =
          reinterpret_cast<vtkUnsignedIntArray*>(callData);
      if (!displayPoints || displayPoints->GetNumberOfTuples() == 0)
      {
        return;
      }
      vtkIdType numPoints = displayPoints->GetNumberOfTuples();
      vtkSmartPointer<vtkPoints> selectionPoints =
          vtkSmartPointer<vtkPoints>::New();
      selectionPoints->SetNumberOfPoints(numPoints);

      vtkRenderer* renderer = this->GetRenderer();
      vtkSmartPointer<vtkCoordinate> coordinate =
          vtkSmartPointer<vtkCoordinate>::New();
      coordinate->SetCoordinateSystemToDisplay();
      double displayPt[3];
      double* worldPt;
      for (vtkIdType i = 0; i < numPoints; ++i)
      {
        displayPoints->GetTuple(i, displayPt);
        coordinate->SetValue(displayPt);
        worldPt = coordinate->GetComputedWorldValue(renderer);
        selectionPoints->SetPoint(i, worldPt[0], worldPt[1], 0.0);
      }

      vtkAnnotatedGraphRepresentation* repr =
          vtkAnnotatedGraphRepresentation::SafeDownCast(
              this->GetRepresentation());
      repr->LassoSelect(selectionPoints, true);
      this->Render();
    }
  }

  this->Superclass::ProcessEvents(caller, eventId, callData);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphView::GenerateAnnotation(void* callData,
    vtkAnnotationLayers* layers)
{
  vtkSmartPointer<vtkAnnotation> annotation =
      vtkSmartPointer<vtkAnnotation>::New();

  vtkSelection* selection = vtkSelection::New();
  this->GenerateSelection(callData, selection);
  annotation->SetSelection(selection);
  selection->Delete();

  double defaultColor[3] =
  { 0.0, 0.0, 1.0 };
  annotation->GetInformation()->Set(vtkAnnotation::COLOR(), defaultColor, 3);
  annotation->GetInformation()->Set(vtkAnnotation::OPACITY(), 0.4);
  annotation->GetInformation()->Set(vtkAnnotation::LABEL(), "New");
  annotation->GetInformation()->Set(vtkAnnotation::ENABLE(), 1);

  layers->AddAnnotation(annotation);
  layers->SetCurrentAnnotation(annotation);
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphView::GenerateSelection(void* callData, vtkSelection* sel)
{
  vtkUnsignedIntArray* points = reinterpret_cast<vtkUnsignedIntArray*>(callData);

  unsigned int pos1X = VTK_UNSIGNED_INT_MAX;
  unsigned int pos1Y = VTK_UNSIGNED_INT_MAX;
  unsigned int pos2X = 0;
  unsigned int pos2Y = 0;
  unsigned int* point = new unsigned int[2];
  for (vtkIdType i = 0; i < points->GetNumberOfTuples(); ++i)
  {
    points->GetTupleValue(i, point);
    if (point[0] < pos1X)
      pos1X = point[0];
    if (point[0] > pos2X)
      pos2X = point[0];
    if (point[1] < pos1Y)
      pos1Y = point[1];
    if (point[1] > pos2Y)
      pos2Y = point[1];
  }
  delete[] point;

  int stretch = 2;
  if (pos1X == pos2X && pos1Y == pos2Y)
  {
    pos1X = pos1X - stretch > 0 ? pos1X - stretch : 0;
    pos1Y = pos1Y - stretch > 0 ? pos1Y - stretch : 0;
    pos2X = pos2X + stretch;
    pos2Y = pos2Y + stretch;
  }
  unsigned int screenMinX = pos1X < pos2X ? pos1X : pos2X;
  unsigned int screenMaxX = pos1X < pos2X ? pos2X : pos1X;
  unsigned int screenMinY = pos1Y < pos2Y ? pos1Y : pos2Y;
  unsigned int screenMaxY = pos1Y < pos2Y ? pos2Y : pos1Y;

  if (this->SelectionMode == FRUSTUM)
  {
    // Do a frustum selection.
    int displayRectangle[4] =
    { screenMinX, screenMinY, screenMaxX, screenMaxY };
    vtkSmartPointer<vtkDoubleArray> frustcorners = vtkSmartPointer<
        vtkDoubleArray>::New();
    frustcorners->SetNumberOfComponents(4);
    frustcorners->SetNumberOfTuples(8);
    //convert screen rectangle to world frustum
    vtkRenderer *renderer = this->GetRenderer();
    double worldP[32];
    int index = 0;
    renderer->SetDisplayPoint(displayRectangle[0], displayRectangle[1], 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[0], displayRectangle[1], 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[0], displayRectangle[3], 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[0], displayRectangle[3], 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[2], displayRectangle[1], 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[2], displayRectangle[1], 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[2], displayRectangle[3], 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);
    index++;
    renderer->SetDisplayPoint(displayRectangle[2], displayRectangle[3], 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&worldP[index * 4]);
    frustcorners->SetTuple4(index, worldP[index * 4], worldP[index * 4 + 1],
        worldP[index * 4 + 2], worldP[index * 4 + 3]);

    vtkSmartPointer<vtkSelectionNode> node =
        vtkSmartPointer<vtkSelectionNode>::New();
    node->SetContentType(vtkSelectionNode::FRUSTUM);
    node->SetFieldType(vtkSelectionNode::CELL);
    node->SetSelectionList(frustcorners);
    sel->AddNode(node);
  }
  else
  {
    this->UpdatePickRender();
    vtkSelection* vsel = this->Selector->GenerateSelection(screenMinX,
        screenMinY, screenMaxX, screenMaxY);
    sel->ShallowCopy(vsel);
    vsel->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkAnnotatedGraphView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
