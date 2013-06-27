/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkOffScreenRepresentation.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkOffScreenRepresentation - represent the vtkOffScreenWidget
//
// .SECTION Description
// vtkOffScreenRepresentation is used to represent a vtkOffScreenWidget. This
// class handles rendering and provides methods for vtkOffScreenWidget to modify
// the appearance of the widget. Most of the hard work is done by the private
// implementation class vtkOffScreenRepresentationImpl.
//
// .SEE ALSO
// vtkOffScreenWidget vtkOffScreenRepresentationImpl

#ifndef __vtkOffScreenRepresentation_h
#define __vtkOffScreenRepresentation_h

#include "vtkcsmWidgetsWin32Header.h"
#include "vtkWidgetRepresentation.h"
#include "vtkSmartPointer.h" // for class ivars

class vtkActor;
class vtkActor2D;
class vtkCellArray;
class vtkCellCenters;
class vtkOffScreenRepresentationImpl;
class vtkPoints;
class vtkProperty2D;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataMapper2D;
class vtkTextActor;
class vtkTextMapper;
class vtkTextProperty;


class VTK_CSM_WIDGETS_EXPORT vtkOffScreenRepresentation:
  public vtkWidgetRepresentation
{
public:
  // Description:
  // Instantiate the class.
  static vtkOffScreenRepresentation *New();

  vtkTypeMacro(vtkOffScreenRepresentation,vtkWidgetRepresentation)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify/retrieve the highlight polydata display when a glyph is
  // selected by a mouse hover.
  void SetHighlightPolyData(vtkPolyData* highlight);
  vtkGetObjectMacro(HighlightPolyData, vtkPolyData)

  // Description:
  // Specify/retrieve the text to display in the balloon.
  vtkGetStringMacro(HoverText)
  vtkSetStringMacro(HoverText)

  // Description:
  // Set/get the text property (relevant only if text is shown).
  void SetTextProperty(vtkTextProperty *p);
  vtkTextProperty* GetTextProperty()
  {
    return this->TextProperty;
  }

  // Description:
  // Set/get the frame property (relevant only if text is shown).
  // The frame lies behind the text.
  void SetFrameProperty(vtkProperty2D *p);
  vtkProperty2D* GetFrameProperty()
  {
    return this->FrameProperty;
  }

  // Description:
  // Set/Get the offset from the mouse pointer from which to place the
  // balloon. The representation will try and honor this offset unless there
  // is a collision with the side of the renderer, in which case the balloon
  // will be repositioned to lie within the rendering window.
  vtkSetVector2Macro(Offset, int)
  vtkGetVector2Macro(Offset, int)

  // Description:
  // Set/Get the padding (in pixels) that is used between the text and the
  // frame.
  vtkSetClampMacro(Padding, int, 0 ,100)
  vtkGetMacro(Padding, int)

  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void StartWidgetInteraction(double e[2]);
  virtual void EndWidgetInteraction(double e[2]);
  virtual void BuildRepresentation();

  // Description:
  // Methods required by vtkProp superclass.
  virtual void ReleaseGraphicsResources(vtkWindow *w);
  virtual int RenderOverlay(vtkViewport *viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport* v);
  virtual int HasTranslucentPolygonalGeometry();

  // Description:
  // Show/hide the flight path highlight.
  void HighlightFlightPath(vtkPoints* itinerary, bool connected);
  void UnHighlightFlightPath();

  // Description:
  // Modify the 'dimness' of the widget.
  void RestoreDim();
  void DimSubtract(double amount);
  void DimMultiply(double amount);

  // Description:
  // Set the landmark centres (cell centers) to represent. The landmarks that
  // are off-screen are determined and displayed by this widget.
  void SetLandmarkCentres(vtkCellCenters* landmarkCentres);

  // Description:
  // Get/Set the colour of the bezel.
  void SetBezelColour(double r, double g, double b, double a);
  void GetBezelColour(double* rgba);

  // Description:
  // Show/hide the representation (connects/disconnects the internal pipeline).
  void SetProjectOffScreenVisibility(bool b);
  bool GetProjectOffScreenVisibility();

  // Description:
  // Get/Set whether to remove overlaps between proxies.
  void SetReduceOverlaps(bool b);
  bool GetReduceOverlaps();

  // Description:
  // When set on in conjunction with Reduce Overlaps, rotate the proxies so that
  // they point back towards the off-screen target.
  void SetShowError(bool b);
  bool GetShowError();

  // Description:
  // Get bits of the representation needed for picking etc. in vtkOffScreenWidget.
  vtkPolyData* GetOffScreenPoly();
  vtkActor2D* GetOffScreenActor();

  // Description:
  // Set the "world size": usually this is the length of the diagonal of the
  // bounding box of the scene in world coordinates. This measure is used to
  // scale the proxies so that their maximum possible length reflects the
  // maximum possible distance of an off-screen target.
  void SetWorldSize(double s);

protected:
  vtkOffScreenRepresentation();
  ~vtkOffScreenRepresentation();

  char* HoverText;
  int Padding;
  int Offset[2];

  // Represent the text
  vtkSmartPointer<vtkTextMapper> TextMapper;
  vtkSmartPointer<vtkActor2D> TextActor;
  vtkTextProperty* TextProperty;

  // Represent the image highlight
  vtkPolyData* HighlightPolyData;
  vtkSmartPointer<vtkPolyDataMapper2D> HighlightMapper;
  vtkSmartPointer<vtkActor2D> HighlightActor;

  // The text frame
  vtkSmartPointer<vtkPoints> FramePoints;
  vtkSmartPointer<vtkCellArray> FramePolygon;
  vtkSmartPointer<vtkPolyData> FramePolyData;
  vtkSmartPointer<vtkPolyDataMapper2D> FrameMapper;
  vtkSmartPointer<vtkActor2D> FrameActor;
  vtkProperty2D* FrameProperty;

  // The flight path
  vtkSmartPointer<vtkPolyData> FlightPathPolyData;
  vtkSmartPointer<vtkPolyDataMapper> FlightPathMapper;
  vtkSmartPointer<vtkActor> FlightPathActor;

  // The offscreen proxies
  vtkCellCenters* LandmarkCentres;
  vtkSmartPointer<vtkOffScreenRepresentationImpl> ProjectOffScreen;
  vtkSmartPointer<vtkPolyDataMapper2D> ProxyMapper;
  vtkSmartPointer<vtkActor2D> ProxyActor;
  vtkSmartPointer<vtkPolyDataMapper2D> BezelMapper;
  vtkSmartPointer<vtkActor2D> BezelActor;
  vtkSmartPointer<vtkPolyData> EmptyPolyData;

  // Internal variables controlling rendering process
  int TextVisible;
  int HighlightVisible;
  int FlightPathVisible;
  int BezelVisible;
  int ProxiesVisible;
  double BezelOpacity;

private:
  vtkOffScreenRepresentation(const vtkOffScreenRepresentation&); //Not implemented
  void operator=(const vtkOffScreenRepresentation&);  //Not implemented
};

#endif
