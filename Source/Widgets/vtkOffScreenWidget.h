/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkOffScreenWidget.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkOffScreenWidget - represent and interact with off-screen objects
//
// .SECTION Description
// The widget takes a set of points from a graph (e.g. the centres of a set of
// annotations or landmarks) and determines which of the points are not currently
// visible on screen as a result of pan and zoom transforms. Each off-screen
// target is represented by a proxy. Proxies are placed on a circular 'bezel'
// (see vtkOffScreenRepresentationImpl) and point towards their off-screen
// counterparts.
//
// Hovering over a proxy highlights it and causes a text label to be displayed.
// Clicking on a proxy initiates a <i>flight</i>: an animated camera action
// that moves the camera along graph edges to reach the selected off-screen
// target. The route is calculated using a vtkFlightMapFilter that must be set
// on this widget. The camera path is calculated so that at the mid-point of the
// animation the zoom is level is such that the entire path can be viewed. To
// facilitate this the followed path is highlighted.
//
// The design of this class is based on vtkHoverWidget.
//
// .SECTION Event Bindings
// By default, the widget observes the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
//   MouseMoveEvent - manages timers used to determine whether the mouse
//                    is hovering on the widget or stopped
//   TimerEvent - when the time between events (e.g., mouse move), then a
//                timer event is invoked.
//   KeyPressEvent - arrow keys are used to move back and forth between points
//                   in the flight history. Escape cancels a flight.
// </pre>
//
// .SECTION See Also
// vtkAbstractWidget vtkFlightMapFilter vtkOffScreenRepresentation

#ifndef __vtkOffScreenWidget_h
#define __vtkOffScreenWidget_h

#include "vtkcsmWidgetsWin32Header.h"
#include "vtkAbstractWidget.h"
#include "vtkSmartPointer.h" // for class ivars

// Private class used to store flight path nodes during animation for
// later playback.
class FlightHistory;

class vtkAbstractPropPicker;
class vtkCellCenters;
class vtkCellLocator;
class vtkFlightMapFilter;
class vtkKdTree;
class vtkOffScreenRepresentation;
class vtkPoints;


class VTK_CSM_WIDGETS_EXPORT vtkOffScreenWidget: public vtkAbstractWidget
{
public:
  static vtkOffScreenWidget *New();
  vtkTypeMacro(vtkOffScreenWidget,vtkAbstractWidget)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the hovering interval (in milliseconds). If after moving the
  // mouse the pointer stays over a vtkProp for this duration, then a
  // vtkTimerEvent::TimerEvent is invoked.
  vtkSetClampMacro(StopTimerDuration, int, 1, 100000)
  vtkGetMacro(StopTimerDuration, int)

  // Description:
  // Specify the dimming interval (in milliseconds). If after moving the
  // mouse the pointer stays over a vtkProp for this duration, then a
  // vtkTimerEvent::TimerEvent is invoked.
  vtkSetClampMacro(DimmerTimerDuration, int, 1, 100000)
  vtkGetMacro(DimmerTimerDuration, int)

  // Description:
  // Specify the interval between each step in an animated 'flight'.
  vtkSetClampMacro(FlightTimerDuration, int, 1, 100000)
  vtkGetMacro(FlightTimerDuration, int)

  // Description:
  // Specify the maximum number of flight legs to remember.
  vtkSetClampMacro(HistoryBufferLength, int, 1, 5000)
  vtkGetMacro(HistoryBufferLength, int)

  // Description:
  // The method for activiating and deactiviating this widget. This method
  // must be overridden because it performs special timer-related operations.
  virtual void SetEnabled(int);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkOffScreenRepresentation* r)
  {
    this->Superclass::SetWidgetRepresentation(
        reinterpret_cast<vtkWidgetRepresentation*>(r));
  }

  // Description:
  // Return the representation as a vtkOffScreenRepresentation.
  vtkOffScreenRepresentation *GetOffScreenRepresentation()
  {
    return reinterpret_cast<vtkOffScreenRepresentation*>(this->WidgetRep);
  }

  // Description:
  // A default representation is created. Note
  // that the superclasses vtkAbstractWidget::GetRepresentation()
  // method returns NULL.
  void CreateDefaultRepresentation();

  // Description:
  // Set/Get the object used to perform pick operations. Since the
  // vtkOffScreenWidget operates on vtkProps, the picker must be a subclass of
  // vtkAbstractPropPicker. (Note: if not specified, an instance of
  // vtkPropPicker is used.)
  void SetPicker(vtkAbstractPropPicker*);
  vtkGetObjectMacro(Picker, vtkAbstractPropPicker)

  // Description:
  // Set the input landmark centers.
  void SetLandmarkCentres(vtkCellCenters* cellCenters);

  // Description:
  // Set the flight map (determines routes between landmarks).
  void SetFlightMap(vtkFlightMapFilter* flightMap);
  vtkGetObjectMacro(FlightMap, vtkFlightMapFilter)

  // Description:
  // Set the type of flight to Express or Tourist.
  void SetFlightType(int flightType);vtkGetMacro(FlightType, int);
  void SetFlightTypeToExpress();
  void SetFlightTypeToTourist();

  // Description:
  // The flight speed bias is added to the timer duration between points along
  // a camera flight, with the effect of slowing it down for positive values and
  // vice versa. Clamped to values -75 <= bias <= 75.
  vtkSetClampMacro(FlightSpeedBias, int, -75, 75)
  vtkGetMacro(FlightSpeedBias, int)

  // Description:
  // When set the representation is dimmed (opacity is reduced) if the mouse
  // stops moving for a few seconds, unless the mouse pointer is hovering over
  // any part of the representation (bezel or proxies).
  vtkSetMacro(AutoDim, bool)
  vtkGetMacro(AutoDim, bool)
  vtkBooleanMacro(AutoDim, bool)

  // Description:
  // Set the colour of the bezel.
  void SetBezelColour(double r, double g, double b, double a);
  void GetBezelColour(double* rgba);

  // Description:
  // Show/hide the representation.
  void SetProjectOffScreen(bool b);
  bool GetProjectOffScreen();

  // Description:
  // Move proxies to remove overlaps.
  void SetReduceOverlaps(bool b);
  bool GetReduceOverlaps();

  // Description:
  // Show the error induced by removing overlaps.
  void SetShowError(bool b);
  bool GetShowError();

  // Description:
  // Set the "world size", the diagonal of the bounding box of the underlying
  // scene in world coordinates. Used to calculate the length of proxy pointers.
  void SetWorldSize(double s);

protected:
  vtkOffScreenWidget();
  ~vtkOffScreenWidget();

  // The state of the widget
  enum
  {
    Start = 0, Stopped, Moving, Flying, Hovering
  };
  int WidgetState;

  // The current preset
  enum
  {
    Express = 0, Tourist
  };
  int FlightType;

  // Callback interface to execute events
  static void LeaveAction(vtkAbstractWidget* w);
  static void MoveAction(vtkAbstractWidget* w);
  static void TimerStoppedAction(vtkAbstractWidget* w);
  static void SelectAction(vtkAbstractWidget* w);
  static void CancelAction(vtkAbstractWidget* w);
  static void HistoryBackAction(vtkAbstractWidget* w);
  static void HistoryForwardAction(vtkAbstractWidget* w);
  int EndTimerAction();

  // Respond to events
  int DoStopped();
  int DoHoverTest();
  int DoDimmer();
  int DoMove();
  int DoSelect();

  void DoFlight(double* arrivalPoint);
  bool CalculateFlightPath(vtkIdType& startVertexId, vtkIdType& endVertexId,
      double* startPoint, double* endPoint, vtkPoints* itinerary);
  void MoveCameraTo(const double& x, const double& y,
      const double& parallelScale);

  // Timer handles and durations.
  int StopTimerId;
  int StopTimerDuration;
  int HoverTimerId;
  int HoverTimerDuration;
  int DimmerTimerId;
  int DimmerTimerDuration;
  int FlightTimerId;
  int FlightTimerDuration;

  // Control the dimming animation.
  bool AutoDim;
  double StartDimness;
  double StepDimness;
  int NumDimSteps;
  int DimStepCount;

  // Speed up/slow down the camera animation speed.
  int FlightSpeedBias;

  // Flight map, route, and history.
  vtkFlightMapFilter* FlightMap;
  FlightHistory* History;
  vtkSmartPointer<vtkPoints> Itinerary;
  vtkIdType CurrentItineraryIndex;
  vtkIdType LastItineraryIndex;
  int HistoryBufferLength;

  // Picking
  vtkAbstractPropPicker* Picker;
  vtkSmartPointer<vtkCellLocator> CellLocator;
  vtkSmartPointer<vtkKdTree> PointLocator;

private:
  vtkOffScreenWidget(const vtkOffScreenWidget&); //Not implemented
  void operator=(const vtkOffScreenWidget&); //Not implemented
};

#endif
