/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkOffScreenWidget.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkActor2D.h"
#include "vtkOffScreenWidget.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include "vtkEvent.h"
#include "vtkFlightMapFilter.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkKdTree.h"
#include "vtkKochanekSpline.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOffScreenRepresentation.h"
#include "vtkOutEdgeIterator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkProp.h"
#include "vtkPropPicker.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStdString.h"
#include "vtkStringArray.h"
#include "vtkTupleInterpolator.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"

#include <algorithm>
#include <deque>
#include <map>


//-------------------------------------------------------------------------
struct HistoryElement
{
  HistoryElement(double x = 0.0, double y = 0.0, double z = 0.0) :
      x(x), y(y), z(z) { }

  //-------------------------------------------------------------------------
  HistoryElement(double* p)
  {
    if (p)
    {
      this->x = p[0];
      this->y = p[1];
      this->z = p[2];
    }
  }
  double x;
  double y;
  double z;
};

//-------------------------------------------------------------------------
class FlightHistory: public std::deque<HistoryElement>
{
public:
  FlightHistory()
  {
    this->Max = 0;
    this->Current = -1;
    this->Front = -1;
  }

  //-------------------------------------------------------------------------
  void SetMaxHistory(const int& max)
  {
    this->Max = max;
  }

  //-------------------------------------------------------------------------
  void AddElement(const HistoryElement& element)
  {
    if (this->Front == this->Max - 1)
    {
      this->pop_front();
    }
    this->push_back(element);
    ++this->Current;
    this->Front = this->Current;
  }

  //-------------------------------------------------------------------------
  HistoryElement GetHistoryForwards()
  {
    if (this->Current < this->Front)
    {
      ++this->Current;
    }
    return this->at(this->Current);
  }

  //-------------------------------------------------------------------------
  HistoryElement GetHistoryBackwards()
  {
    if (this->Current > 0)
    {
      --this->Current;
    }
    return this->at(this->Current);
  }

private:
  int Max;
  int Current;
  int Front;
};

//-------------------------------------------------------------------------
vtkStandardNewMacro(vtkOffScreenWidget)
vtkCxxSetObjectMacro(vtkOffScreenWidget, FlightMap, vtkFlightMapFilter)

//-------------------------------------------------------------------------
vtkOffScreenWidget::vtkOffScreenWidget()
{
  this->WidgetState = Start;
  this->HoverTimerDuration = 500;
  this->HoverTimerId = -1;
  this->StopTimerDuration = 3000;
  this->StopTimerId = -1;
  this->DimmerTimerDuration = 40;
  this->DimmerTimerId = -1;
  this->FlightTimerDuration = 100;
  this->FlightTimerId = -1;

  this->StartDimness = 0.25;
  this->StepDimness = 0.03;
  this->NumDimSteps = 50;
  this->AutoDim = false;
  this->DimStepCount = 0;

  this->Picker = vtkPropPicker::New();
  this->CellLocator = vtkSmartPointer<vtkCellLocator>::New();
  this->CellLocator->SetTolerance(2.0);
  this->CellLocator->SetNumberOfCellsPerBucket(1);
  this->PointLocator = vtkSmartPointer<vtkKdTree>::New();
  this->PointLocator->OmitZPartitioning();
  this->PointLocator->SetTolerance(0.0001);

  this->Itinerary = vtkSmartPointer<vtkPoints>::New();
  this->FlightMap = 0;
  this->LastItineraryIndex = -1;
  this->CurrentItineraryIndex = -1;
  this->FlightType = 0;
  this->FlightSpeedBias = 0;
  this->HistoryBufferLength = 250;
  this->History = new FlightHistory;
  this->History->SetMaxHistory(this->HistoryBufferLength);

  // Okay, define the events for this widget. Note that we look for extra events
  // (like button press) because without it the hover widget thinks nothing has changed
  // and doesn't begin retiming.
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeaveEvent,
      vtkWidgetEvent::Reset, this, vtkOffScreenWidget::LeaveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
      vtkWidgetEvent::Select, this, vtkOffScreenWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
      vtkWidgetEvent::Move, this, vtkOffScreenWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
      vtkWidgetEvent::Move, this, vtkOffScreenWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseWheelForwardEvent,
      vtkWidgetEvent::Move, this, vtkOffScreenWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseWheelBackwardEvent,
      vtkWidgetEvent::Move, this, vtkOffScreenWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
      vtkWidgetEvent::Move, this, vtkOffScreenWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::TimerEvent,
      vtkWidgetEvent::TimedOut, this, vtkOffScreenWidget::TimerStoppedAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
      vtkEvent::AnyModifier, 27, 1, "Escape", vtkWidgetEvent::Completed, this,
      vtkOffScreenWidget::CancelAction);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
      vtkEvent::AnyModifier, 29, 1, "Left", vtkWidgetEvent::Scale, this,
      vtkOffScreenWidget::HistoryBackAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent,
      vtkEvent::AnyModifier, 28, 1, "Right", vtkWidgetEvent::Rotate, this,
      vtkOffScreenWidget::HistoryForwardAction);
  this->KeyPressActivationOff();

  // Create the representation up front so that we can set some of its ivars.
  this->CreateDefaultRepresentation();
}

//-------------------------------------------------------------------------
vtkOffScreenWidget::~vtkOffScreenWidget()
{
  this->SetFlightMap(0);
  this->SetInteractor(0);
  delete this->History;
  this->Picker->Delete();
  this->SetWidgetRepresentation(0);
}

//----------------------------------------------------------------------
void vtkOffScreenWidget::SetLandmarkCentres(vtkCellCenters* landmarkCentres)
{
  reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)
      ->SetLandmarkCentres(landmarkCentres);
}

//----------------------------------------------------------------------
void vtkOffScreenWidget::SetEnabled(int enabling)
{
  if (enabling)
  {
    vtkDebugMacro(<<"Enabling widget");

    if (this->Enabled) //already enabled, just return
    {
      return;
    }

    if (!this->Interactor)
    {
      vtkErrorMacro(<<"The interactor must be set prior to enabling the widget");
      return;
    }

    // We're ready to enable
    this->Enabled = 1;

    // listen for the events found in the EventTranslator
    this->EventTranslator->AddEventsToInteractor(this->Interactor,
        this->EventCallbackCommand, this->Priority);

    // Start off the timer
    this->StopTimerId = this->Interactor->CreateRepeatingTimer(
        this->StopTimerDuration);
    this->WidgetState = vtkOffScreenWidget::Stopped;

    this->InvokeEvent(vtkCommand::EnableEvent, NULL);

    this->SetCurrentRenderer(
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    if (this->CurrentRenderer)
    {
      // Build the representation
      this->WidgetRep->SetRenderer(this->CurrentRenderer);
      this->WidgetRep->BuildRepresentation();
      this->CurrentRenderer->AddViewProp(this->WidgetRep);
    }
  }

  else //disabling------------------
  {
    vtkDebugMacro(<<"Disabling widget");

    if (!this->Enabled) //already disabled, just return
    {
      return;
    }

    this->Enabled = 0;
    this->Interactor->RemoveObserver(this->EventCallbackCommand);
    this->InvokeEvent(vtkCommand::DisableEvent, NULL);
    if (this->CurrentRenderer)
    {
      this->CurrentRenderer->RemoveViewProp(this->WidgetRep);
      this->SetCurrentRenderer(NULL);
    }
  }
}

//----------------------------------------------------------------------
void vtkOffScreenWidget::SetPicker(vtkAbstractPropPicker *picker)
{
  if (picker == NULL || picker == this->Picker)
  {
    return;
  }

  this->Picker->Delete();
  this->Picker = picker;
  this->Picker->Register(this);
  this->Modified();
}

//----------------------------------------------------------------------
void vtkOffScreenWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkOffScreenRepresentation::New();
  }
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::LeaveAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  self->Interactor->DestroyTimer(self->StopTimerId);
  self->Interactor->DestroyTimer(self->HoverTimerId);
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::MoveAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  if (self->WidgetState == vtkOffScreenWidget::Stopped)
  {
    // Started moving
    self->DoMove();
    self->Interactor->DestroyTimer(self->StopTimerId);
    self->Interactor->DestroyTimer(self->DimmerTimerId);
    self->InvokeEvent(vtkCommand::WidgetActivateEvent, NULL);
  }
  else
  {
    // Still moving
    self->Interactor->DestroyTimer(self->StopTimerId);
  }
  self->StopTimerId = self->Interactor->CreateRepeatingTimer(
      self->StopTimerDuration);
  self->HoverTimerId = self->Interactor->CreateRepeatingTimer(
      self->HoverTimerDuration);
  if (self->WidgetState != vtkOffScreenWidget::Flying)
  {
    self->WidgetState = vtkOffScreenWidget::Moving;
  }
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::TimerStoppedAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  int timerId = *(reinterpret_cast<int*>(self->CallData));

  if (timerId == self->HoverTimerId
      && self->WidgetState != vtkOffScreenWidget::Flying)
  {
    // Test for hover
    if (self->DoHoverTest())
    {
      self->WidgetState = vtkOffScreenWidget::Hovering;
      self->Interactor->DestroyTimer(self->StopTimerId);
    }
    self->Interactor->DestroyTimer(self->HoverTimerId);
    self->InvokeEvent(vtkCommand::TimerEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1);
  }

  if (timerId == self->StopTimerId
      && self->WidgetState == vtkOffScreenWidget::Moving)
  {
    // Stopped (i.e. not moved for the stop timer duration)
    self->Interactor->DestroyTimer(self->StopTimerId);
    self->WidgetState = vtkOffScreenWidget::Stopped;
    self->DoStopped();
    self->InvokeEvent(vtkCommand::TimerEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1);

    if (self->AutoDim)
    {
      self->DimmerTimerId = self->Interactor->CreateRepeatingTimer(
          self->DimmerTimerDuration);
      self->DimStepCount = 0;
    }
  }

  if (timerId == self->DimmerTimerId
      && self->WidgetState == vtkOffScreenWidget::Stopped)
  {
    // The dimmer timer timed out and we aren't moving
    if (self->DimStepCount > self->NumDimSteps)
    {
      // Stop the dimmer timer
      self->Interactor->DestroyTimer(self->DimmerTimerId);
      self->InvokeEvent(vtkCommand::TimerEvent, NULL);
      self->EventCallbackCommand->SetAbortFlag(1);
    }
    else
    {
      // Do the next dim step
      self->DoDimmer();
      ++self->DimStepCount;
      self->InvokeEvent(vtkCommand::TimerEvent, NULL);
      self->EventCallbackCommand->SetAbortFlag(1);
    }
  }

  if (timerId == self->FlightTimerId
      && self->WidgetState == vtkOffScreenWidget::Flying)
  {
    // The flight timer timed out, take the next step
    double* p = self->Itinerary->GetPoint(self->CurrentItineraryIndex);
    self->History->AddElement(HistoryElement(p[0], p[1], p[2]));
    self->MoveCameraTo(p[0], p[1], p[2]);
    ++self->CurrentItineraryIndex;

    if (self->CurrentItineraryIndex == self->LastItineraryIndex)
    {
      // That was the last leg of the flight
      self->Interactor->DestroyTimer(self->FlightTimerId);
      self->WidgetState = vtkOffScreenWidget::Stopped;
      // Undraw the vapour trail
      reinterpret_cast<vtkOffScreenRepresentation*>(self->WidgetRep)
          ->UnHighlightFlightPath();
    }
    self->InvokeEvent(vtkCommand::TimerEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1);
  }
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SelectAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);

  // If widget is hovering we grab the selection event
  if (self->WidgetState != vtkOffScreenWidget::Flying)
  {
    self->DoSelect();
    self->InvokeEvent(vtkCommand::WidgetActivateEvent, NULL);
  }
}

//-------------------------------------------------------------------------
int vtkOffScreenWidget::DoDimmer()
{
  vtkOffScreenRepresentation* widgetRep =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  widgetRep->DimMultiply(1.0 - this->StepDimness);
  this->Render();
  return 1;
}

//-------------------------------------------------------------------------
int vtkOffScreenWidget::DoMove()
{
  // undim the bezel/proxies
  vtkOffScreenRepresentation* widgetRep =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);

  widgetRep->RestoreDim();
  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  widgetRep->StartWidgetInteraction(e);

  this->Render();
  return 1;
}

//-------------------------------------------------------------------------
// If the mouse is over the bezel do nothing.
// If the mouse is over a proxy, set the label and highlight on the repr.
// Otherwise, clear the higlight and label, and dim the bezel
int vtkOffScreenWidget::DoStopped()
{
  this->EndTimerAction();
  return 1;
}

//-------------------------------------------------------------------------
int vtkOffScreenWidget::DoHoverTest()
{
  vtkOffScreenRepresentation* widgetRep =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  vtkPolyData* offScreenPoly = widgetRep->GetOffScreenPoly();

  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  vtkRenderer *ren = this->CurrentRenderer;
  this->Picker->Pick(e[0], e[1], 0.0, ren);
  vtkProp* pickedProp = this->Picker->GetViewProp();
  if (pickedProp == this->WidgetRep)
  {
    double p1[3] =
    { e[0], e[1], -0.1 };
    double p2[3] =
    { e[0], e[1], 0.1 };
    vtkIdList* hits = vtkIdList::New();
    vtkIdType cellId;

    if (offScreenPoly->GetNumberOfCells() > 0)
    {
      this->CellLocator->SetDataSet(offScreenPoly);
      this->CellLocator->BuildLocator();
      this->CellLocator->FindCellsAlongLine(p1, p2, 0.0, hits);
    }

    if (hits->GetNumberOfIds() > 0)
    {
      // Picking a proxy
      cellId = hits->GetId(hits->GetNumberOfIds() - 1);
      vtkStringArray* labels = vtkStringArray::SafeDownCast(
          offScreenPoly->GetCellData()->GetAbstractArray("Hull name"));
      vtkPolyData* wedgie = vtkPolyData::New();
      vtkIdTypeArray* clusterIds = vtkIdTypeArray::SafeDownCast(
          offScreenPoly->GetCellData()->GetAbstractArray("Hull id"));
      if (clusterIds)
      {
        int clusterId = clusterIds->GetValue(cellId);
        vtkCellArray* polys = vtkCellArray::New();

        // Find all cells with this wedge id and add to highlight polydata.
        int n = offScreenPoly->GetNumberOfCells();
        for (int i = 0; i < n; ++i)
        {
          if (clusterIds->GetValue(i) == clusterId)
          {
            polys->InsertNextCell(offScreenPoly->GetCell(i));
          }
        }

        wedgie->SetPoints(offScreenPoly->GetPoints());
        wedgie->SetPolys(polys);
        polys->Delete();
      }

      if (labels)
      {
        reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHoverText(
            labels->GetValue(cellId));
      }

      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHighlightPolyData(
          wedgie);
      this->WidgetRep->StartWidgetInteraction(e);
      this->Render();
      wedgie->Delete();
    }
    else
    {
      // Picking the bezel
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHoverText(
          "");
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHighlightPolyData(
          0);
      this->Render();
    }
    hits->Delete();
  }
  else
  {
    // Not picking the widget at all
    reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHoverText(
        "");
    reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->SetHighlightPolyData(
        0);
    this->EndTimerAction();
    this->Render();
    return 0;
  }

  return 1;
}

//-------------------------------------------------------------------------
int vtkOffScreenWidget::EndTimerAction()
{
  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  this->WidgetRep->EndWidgetInteraction(e);
  this->Render();

  return 1;
}

//-------------------------------------------------------------------------
int vtkOffScreenWidget::DoSelect()
{
  vtkOffScreenRepresentation* widgetRep =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  if (widgetRep->GetOffScreenActor() == NULL)
  {
    vtkErrorMacro(<< "No ProjectOffScreenProp/Proxies set.");
    return 0;
  }

  if (this->FlightMap == NULL)
  {
    vtkErrorMacro(<< "No flight map filter set.");
    return 0;
  }

  vtkPolyData* offScreenPoly = widgetRep->GetOffScreenPoly();
  if (offScreenPoly->GetNumberOfCells() == 0)
  {
    vtkErrorMacro(<< "Off-screen polydata is empty.");
    return 0;
  }

  this->Picker->AddPickList(widgetRep->GetOffScreenActor());
  this->CellLocator->SetDataSet(offScreenPoly);
  this->CellLocator->BuildLocator();

  double e[2];
  e[0] = static_cast<double>(this->Interactor->GetEventPosition()[0]);
  e[1] = static_cast<double>(this->Interactor->GetEventPosition()[1]);
  this->Picker->Pick(e[0], e[1], 0.0, this->CurrentRenderer);
  vtkAssemblyPath* path = this->Picker->GetPath();
  if (path != NULL)
  {
    double p1[3] = { e[0], e[1], -0.1 };
    double p2[3] = { e[0], e[1], 0.1 };
    vtkSmartPointer<vtkIdList> hits = vtkSmartPointer<vtkIdList>::New();
    vtkIdType cellId;
    this->CellLocator->FindCellsAlongLine(p1, p2, 0.0, hits);

    if (hits->GetNumberOfIds() > 0)
    {
      cellId = hits->GetId(hits->GetNumberOfIds() - 1);
      vtkDoubleArray* points = vtkDoubleArray::SafeDownCast(
          offScreenPoly->GetCellData()->GetAbstractArray("Hull point"));
      if (points)
      {
        this->WidgetRep->StartWidgetInteraction(e);
        double arrivalPoint[3];
        points->GetTupleValue(cellId, arrivalPoint);
        this->DoFlight(arrivalPoint);
        this->WidgetState = vtkOffScreenWidget::Flying;
      }
      else
      {
        vtkWarningMacro(<< "No landmark points in vtkProjectOffScreenProxies");
        this->Picker->DeletePickList(widgetRep->GetOffScreenActor());
        return 0;
      }
    }
  }
  this->Picker->DeletePickList(widgetRep->GetOffScreenActor());
  return 1;
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::CancelAction(vtkAbstractWidget *w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  int timerId = *(reinterpret_cast<int*>(self->CallData));

  if (timerId == self->FlightTimerId
      && self->WidgetState == vtkOffScreenWidget::Flying)
  {
    self->Interactor->DestroyTimer(self->FlightTimerId);
    self->WidgetState = vtkOffScreenWidget::Stopped;
    self->InvokeEvent(vtkCommand::TimerEvent, NULL);
    self->EventCallbackCommand->SetAbortFlag(1); //no one else gets this event
  }
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::HistoryBackAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  HistoryElement e = self->History->GetHistoryBackwards();
  self->MoveCameraTo(e.x, e.y, e.z);
  self->InvokeEvent(vtkCommand::WidgetActivateEvent, NULL);
  self->EventCallbackCommand->SetAbortFlag(1); //no one else gets this event
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::HistoryForwardAction(vtkAbstractWidget* w)
{
  vtkOffScreenWidget *self = reinterpret_cast<vtkOffScreenWidget*>(w);
  HistoryElement e = self->History->GetHistoryForwards();
  self->MoveCameraTo(e.x, e.y, e.z);
  self->InvokeEvent(vtkCommand::WidgetActivateEvent, NULL);
  self->EventCallbackCommand->SetAbortFlag(1); //no one else gets this event
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::SetFlightType(int flightType)
{
  switch (flightType)
  {
  case vtkOffScreenWidget::Express:
    this->FlightType = flightType;
    this->FlightTimerDuration = 100;
    this->FlightMap->SetExpressPreset();
    break;
  case vtkOffScreenWidget::Tourist:
    this->FlightType = flightType;
    this->FlightTimerDuration = 125;
    this->FlightMap->SetTouristPreset();
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::SetFlightTypeToExpress()
{
  this->SetFlightType(vtkOffScreenWidget::Express);
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::SetFlightTypeToTourist()
{
  this->SetFlightType(vtkOffScreenWidget::Tourist);
}

//----------------------------------------------------------------------------
// Functor to compare distances in Dijkstra's SSSP, used in CalculateFlightPath
struct vtkOffScreenWidgetCompareDistanceMap
{
  bool operator()(const std::pair<vtkIdType, double>& left,
      const std::pair<vtkIdType, double>& right) const
  {
    return left.second < right.second;
  }
} CompareDistance;

//----------------------------------------------------------------------------
bool vtkOffScreenWidget::CalculateFlightPath(vtkIdType& startVertexId,
    vtkIdType& endVertexId, double* startPoint, double* endPoint,
    vtkPoints* itinerary)
{
  std::map<vtkIdType, double> distance;
  std::map<vtkIdType, vtkIdType> predecessor;
  vtkOutEdgeIterator* edges = vtkOutEdgeIterator::New();
  vtkGraph* flightMap = this->FlightMap->GetOutput();
  vtkIdType numVertices = flightMap->GetNumberOfVertices();
  vtkDoubleArray* weight = vtkDoubleArray::SafeDownCast(
      flightMap->GetEdgeData()->GetAbstractArray("weights"));

  vtkTupleInterpolator* interpolator = vtkTupleInterpolator::New();
  interpolator->SetInterpolationTypeToLinear();
  interpolator->SetNumberOfComponents(3);

  // Dijkstra's SSSP
  for (vtkIdType v = 0; v < numVertices; ++v)
  {
    distance[v] = VTK_DOUBLE_MAX;
    predecessor[v] = -1.0;
  }

  distance[startVertexId] = 0.0;
  while (!distance.empty())
  {
    std::pair<vtkIdType, double> minDist = *std::min_element(distance.begin(),
        distance.end(), CompareDistance);
    vtkIdType u = minDist.first;
    if (distance[u] == VTK_DOUBLE_MAX)
    {
      // There is no path
        interpolator->AddTuple(0.0, startPoint);
        interpolator->AddTuple(1.0, endPoint);
        double p[3];
        for (int i = 0; i <= 4; i++)
        {
          interpolator->InterpolateTuple(0.25 * i, p);
          itinerary->InsertNextPoint(p);
        }
        edges->Delete();
        interpolator->Delete();
        return false;
      }
    if (u == endVertexId)
    {
      break; // found the target
    }

    flightMap->GetOutEdges(u, edges);
    while (edges->HasNext())
    {
      vtkOutEdgeType edge = edges->Next();
      vtkIdType v = edge.Target;
      if (distance.count(v) == 0)
        continue;

      if (distance[u] + weight->GetValue(edge.Id) < distance[v])
      {
        distance[v] = distance[u] + weight->GetValue(edge.Id);
        predecessor[v] = u;
      }
    }
    distance.erase(u);
  }
  edges->Delete();

  // Gather up the results
  vtkIdType u = endVertexId;
  vtkIdList* reverseItinerary = vtkIdList::New();
  while (predecessor[u] >= 0)
  {
    reverseItinerary->InsertNextId(u);
    u = predecessor[u];
  }
  reverseItinerary->InsertNextId(startVertexId);

  // UnReverse the list
  vtkIdType numIds = reverseItinerary->GetNumberOfIds();
  vtkIdList* forwardItinerary = vtkIdList::New();
  for (vtkIdType i = numIds - 1; i >= 0; --i)
  {
    forwardItinerary->InsertNextId(reverseItinerary->GetId(i));
  }

  // Get the point
  itinerary->InsertNextPoint(flightMap->GetPoint(forwardItinerary->GetId(0)));

  for (vtkIdType i = 1; i < numIds; ++i)
  {
    vtkIdType currentEdge = flightMap->GetEdgeId(forwardItinerary->GetId(i - 1),
        forwardItinerary->GetId(i));
    vtkIdType npts;
    double* pts;
    flightMap->GetEdgePoints(currentEdge, npts, pts);

    double* vertexPoint = flightMap->GetPoint(forwardItinerary->GetId(i - 1));
    itinerary->InsertNextPoint(vertexPoint);

    if (npts > 0)
    {
      interpolator->Initialize();
      interpolator->SetNumberOfComponents(3);
      interpolator->SetInterpolationTypeToLinear();
      double p[3];
      for (vtkIdType j = 0; j < npts; ++j)
      {
        p[0] = pts[j * 3 + 0];
        p[1] = pts[j * 3 + 1];
        p[2] = pts[j * 3 + 2];
        interpolator->AddTuple((1.0 / (npts - 1)) * j, p);
      }
      if (pts[0] == vertexPoint[0] && pts[1] == vertexPoint[1])
      {
        for (int j = 1; j <= 3; j++)
        {
          interpolator->InterpolateTuple(0.25 * j, p);
          itinerary->InsertNextPoint(p);
        }
      }
      else
      {
        for (int j = 3; j <= 1; j--)
        {
          interpolator->InterpolateTuple(0.25 * j, p);
          itinerary->InsertNextPoint(p);
        }
      }
    }
  }
  itinerary->InsertNextPoint(
      flightMap->GetPoint(forwardItinerary->GetId(numIds - 1)));

  reverseItinerary->Delete();
  forwardItinerary->Delete();
  interpolator->Delete();
  return true;
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::DoFlight(double* arrivalPoint)
{
  // Find start (display centre) point in world coords
  double departurePoint[3];
  int* size = this->Interactor->GetSize();
  this->ComputeDisplayToWorld(size[0] / 2.0, size[1] / 2.0, 0.0,
      departurePoint);
  departurePoint[2] = 0.0;
  arrivalPoint[2] = 0.0;

  // Find nearest vertex to start point - subset the map points so the flight at
  // least starts off in roughly the right direction
  double theta = vtkMath::Pi() / 2.0;
  double direction[3] =
  { (arrivalPoint[0] - departurePoint[0]),
      (arrivalPoint[1] - departurePoint[1]), 0.0 };
  double A[3] =
  { departurePoint[0] + (direction[0] * cos(theta) - direction[1] * sin(theta)),
      departurePoint[1]
          + (direction[0] * sin(theta) + direction[1] * cos(theta)), 0.0 };
  double B[3] =
  { departurePoint[0] + (direction[0] * cos(theta) + direction[1] * sin(theta)),
      departurePoint[1]
          + (direction[0] * -sin(theta) + direction[1] * cos(theta)), 0.0 };

  double side = ((B[0] - A[0]) * (arrivalPoint[1] - A[1]))
      - ((B[1] - A[1]) * (arrivalPoint[0] - A[0]));

  this->FlightMap->Update();
  vtkGraph* flightMap = this->FlightMap->GetOutput();
  vtkPoints* mapPoints = flightMap->GetPoints();
  vtkIdType numMapPoints = mapPoints->GetNumberOfPoints();
  vtkPoints* subsettedPoints = vtkPoints::New();
  std::map<vtkIdType, vtkIdType> subsetToSetMap;
  double C[3];
  for (vtkIdType i = 0; i < numMapPoints; ++i)
  {
    mapPoints->GetPoint(i, C);
    if ((((B[0] - A[0]) * (C[1] - A[1])) - ((B[1] - A[1]) * (C[0] - A[0])) <= 0)
        == (side <= 0))
    {
      subsetToSetMap[subsettedPoints->InsertNextPoint(C)] = i;
    }
  }

  this->PointLocator->BuildLocatorFromPoints(subsettedPoints);
  double distance;
  vtkIdType closestToStart =
      subsetToSetMap[this->PointLocator->FindClosestPoint(departurePoint,
          distance)];
  subsettedPoints->Delete();

  // Find nearest vertex to arrival point
  this->PointLocator->BuildLocatorFromPoints(mapPoints);
  vtkIdType closestToEnd = this->PointLocator->FindClosestPoint(arrivalPoint,
      distance);

  // Calculate flight itinerary
  vtkPoints* itinerary = vtkPoints::New();
  bool connected = this->CalculateFlightPath(closestToStart, closestToEnd,
      departurePoint, arrivalPoint, itinerary);

  // Taxi to itinerary start point
  vtkPoints* taxiPath = vtkPoints::New();
  taxiPath->InsertNextPoint(departurePoint);
  taxiPath->InsertNextPoint(itinerary->GetPoint(0));
  reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)
      ->HighlightFlightPath(taxiPath, false);
  this->MoveCameraTo(itinerary->GetPoint(0)[0], itinerary->GetPoint(0)[1],
      this->CurrentRenderer->GetActiveCamera()->GetParallelScale());
  reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->
      UnHighlightFlightPath();
  taxiPath->Delete();

  // Draw con-trail
  reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep)->HighlightFlightPath(
      itinerary, connected);

  // Amount of zoom depends on distance between start and end as proportion of world
  double flightDistance = sqrt(
      (pow(arrivalPoint[0] - departurePoint[0], 2))
          + (pow(arrivalPoint[1] - departurePoint[1], 2)));
  double* worldBounds = this->CurrentRenderer->ComputeVisiblePropBounds();
  double worldSize = sqrt(
      (pow(worldBounds[1] - worldBounds[0], 2))
          + (pow(worldBounds[3] - worldBounds[2], 2)));
  double startZoom =
      this->CurrentRenderer->GetActiveCamera()->GetParallelScale();
  double maxZoom = (flightDistance / worldSize)
      * (worldBounds[3] - worldBounds[2]);
  if (maxZoom < startZoom)
  {
    maxZoom = startZoom;
  }

  // Smooth the zoom path
  vtkIdType numItineraryPoints = itinerary->GetNumberOfPoints();
  vtkKochanekSpline* zoomSpline = vtkKochanekSpline::New();
  zoomSpline->SetDefaultBias(0.0);
  zoomSpline->SetDefaultTension(0.0);
  zoomSpline->SetDefaultContinuity(0.0);
  zoomSpline->SetClosed(false);
  vtkTupleInterpolator* zoomInterpolator = vtkTupleInterpolator::New();
  zoomInterpolator->SetNumberOfComponents(1);
  zoomInterpolator->SetInterpolationTypeToSpline();
  zoomInterpolator->SetInterpolatingSpline(zoomSpline);
  zoomSpline->Delete();
  zoomInterpolator->AddTuple(0.0 * numItineraryPoints, &startZoom);
  zoomInterpolator->AddTuple(0.3333 * numItineraryPoints, &maxZoom);
  zoomInterpolator->AddTuple(0.6667 * numItineraryPoints, &maxZoom);
  zoomInterpolator->AddTuple(1.0 * numItineraryPoints, &startZoom);

  // Smooth the x,y-path
  vtkKochanekSpline* xySpline = vtkKochanekSpline::New();
  xySpline->SetDefaultBias(0.5);
  xySpline->SetDefaultTension(0.25);
  xySpline->SetDefaultContinuity(-1.0);
  xySpline->SetClosed(false);
  vtkTupleInterpolator* xyInterpolator = vtkTupleInterpolator::New();
  xyInterpolator->SetNumberOfComponents(3);
  xyInterpolator->SetInterpolationTypeToSpline();
  xyInterpolator->SetInterpolatingSpline(xySpline);
  xySpline->Delete();
  for (vtkIdType i = 0; i < numItineraryPoints; ++i)
  {
    xyInterpolator->AddTuple(1.0 * i, itinerary->GetPoint(i));
  }

  double resolution = 4.0;
  vtkIdType numSmoothedItineraryPoints = numItineraryPoints * resolution;
  double interpolatedCameraXY[2];
  double interpolatedCameraZoom;
  double interpolationT;
  this->Itinerary->Initialize();

  for (vtkIdType i = 0; i < numSmoothedItineraryPoints; ++i)
  {
    interpolationT = i / resolution;
    zoomInterpolator->InterpolateTuple(interpolationT, &interpolatedCameraZoom);
    xyInterpolator->InterpolateTuple(interpolationT, interpolatedCameraXY);
    this->Itinerary->InsertNextPoint(interpolatedCameraXY[0],
        interpolatedCameraXY[1], interpolatedCameraZoom);
  }

  // Lift off!
  this->FlightTimerId = this->Interactor->CreateRepeatingTimer(
      this->FlightTimerDuration + this->FlightSpeedBias);
  this->WidgetState = vtkOffScreenWidget::Flying;
  this->CurrentItineraryIndex = 0;
  this->LastItineraryIndex = numSmoothedItineraryPoints;
  this->History->AddElement(
      HistoryElement(departurePoint[0], departurePoint[1], startZoom));

  itinerary->Delete();
  zoomInterpolator->Delete();
  xyInterpolator->Delete();
}

//----------------------------------------------------------------------------
void vtkOffScreenWidget::MoveCameraTo(const double& x, const double& y,
    const double& parallelScale)
{
  vtkRenderer* ren = this->GetCurrentRenderer();
  vtkCamera* camera = ren->GetActiveCamera();

  double flyFrom[3];
  double positionFrom[3];
  double flyTo[3];
  camera->GetFocalPoint(flyFrom);
  camera->GetPosition(positionFrom);
  flyTo[0] = x;
  flyTo[1] = y;
  flyTo[2] = flyFrom[2];

  double d[3];
  d[0] = flyTo[0] - flyFrom[0];
  d[1] = flyTo[1] - flyFrom[1];
  d[2] = 0.0;
  double distance = vtkMath::Normalize(d);
  //  int resolution = this->Interactor->GetNumberOfFlyFrames();
  int resolution = 1;
  double delta = distance / resolution;

  double startScale = camera->GetParallelScale();
  double zoomDelta = (parallelScale - startScale) / resolution;
  double scale = startScale;

  double focalPt[3], position[3];
  for (int i = 1; i <= resolution; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      focalPt[j] = flyFrom[j] + d[j] * i * delta;
      position[j] = positionFrom[j] + d[j] * i * delta;
    }
    scale = startScale + i * zoomDelta;
    ren->GetActiveCamera()->SetFocalPoint(focalPt);
    ren->GetActiveCamera()->SetPosition(position);
    ren->GetActiveCamera()->SetParallelScale(scale);
    ren->ResetCameraClippingRange();
    this->Interactor->Render();
  }
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SetBezelColour(double r, double g, double b, double a)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  repr->SetBezelColour(r, g, b, a);
  this->StartDimness = a;
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::GetBezelColour(double* rgba)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  repr->GetBezelColour(rgba);
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SetProjectOffScreen(bool b)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  if (b)
  {
    this->EnabledOn();
    repr->SetProjectOffScreenVisibility(true);
  }
  else
  {
    this->EnabledOff();
    repr->SetProjectOffScreenVisibility(false);
  }
  this->Modified();
}

//-------------------------------------------------------------------------
bool vtkOffScreenWidget::GetProjectOffScreen()
{
  return this->Enabled;
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SetReduceOverlaps(bool b)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  repr->SetReduceOverlaps(b);
}

//-------------------------------------------------------------------------
bool vtkOffScreenWidget::GetReduceOverlaps()
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  return repr->GetReduceOverlaps();
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SetShowError(bool b)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  repr->SetShowError(b);
}

//-------------------------------------------------------------------------
bool vtkOffScreenWidget::GetShowError()
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  return repr->GetShowError();
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::SetWorldSize(double s)
{
  vtkOffScreenRepresentation* repr =
      reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  repr->SetWorldSize(s);
}

//-------------------------------------------------------------------------
void vtkOffScreenWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Stop Timer Duration: " << this->StopTimerDuration << "\n";
  os << indent << "Hover Timer Duration: " << this->HoverTimerDuration << "\n";
  os << indent << "Dimmer Timer Duration: " << this->DimmerTimerDuration << "\n";
  os << indent << "Flight Timer Duration: " << this->FlightTimerDuration << "\n";
  os << indent << "Auto Dim: " << (this->AutoDim ? "On" : "Off") << "\n";
  os << indent << "Flight Speed Bias: " << this->FlightSpeedBias << "\n";
  os << indent << "History Buffer Length: " << this->HistoryBufferLength << "\n";
  os << indent << "Flight Map: " << this->FlightMap << "\n";
  os << indent << "Cell Locator: " << "\n";
  this->CellLocator->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Point Locator: " << "\n";
    this->PointLocator->PrintSelf(os, indent.GetNextIndent());
}
