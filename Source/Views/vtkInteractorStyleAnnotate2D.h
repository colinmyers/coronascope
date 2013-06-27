/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkInteractorStyleAnnotate2D.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkInteractorStyleAnnotate2D - A rubber band interactor for a 2D view
//
// .SECTION Description
// vtkInteractorStyleAnnotate2D manages interaction in a 2D view, such as
// vtkAnnotatedGraphRepresentation.
// Camera rotation is not allowed with this interactor style.
// Zooming affects the camera's parallel scale only, and assumes
// that the camera is in parallel projection mode.
// The style also allows drawing an arbitrary shape lasso/rubber band selection
// using the left button.
//
// All camera changes invoke InteractionBeginEvent when the button
// is pressed, InteractionEvent when the mouse (or wheel) is moved,
// and InteractionEndEvent when the button is released.  The bindings
// are as follows:
// Left mouse - Select (invokes a SelectionChangedEvent).
// Right mouse - Zoom.
// Middle mouse - Pan.
// Scroll wheel - Zoom.
// Delete key - Undo (invokes a UserEvent).

#ifndef __vtkInteractorStyleAnnotate2D_h
#define __vtkInteractorStyleAnnotate2D_h

#include "vtkcsmViewsWin32Header.h"
#include "vtkInteractorStyle.h"

class vtkUnsignedCharArray;
class vtkUnsignedIntArray;


class VTK_CSM_VIEWS_EXPORT vtkInteractorStyleAnnotate2D:
  public vtkInteractorStyle
{
public:
  static vtkInteractorStyleAnnotate2D *New();
  vtkTypeMacro(vtkInteractorStyleAnnotate2D, vtkInteractorStyle)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Respond to events.
  virtual void OnKeyPress();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseMove();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  // Description:
  // Whether to invoke a render when the mouse moves.
  vtkSetMacro(RenderOnMouseMove, bool)
  vtkGetMacro(RenderOnMouseMove, bool)
  vtkBooleanMacro(RenderOnMouseMove, bool)

  //BTX
  // Description:
  // Selection types
  enum
  {
    SELECT_NORMAL = 0, SELECT_UNION = 1
  };
  //ETX

  // Description:
  // Current interaction state
  vtkGetMacro(Interaction, int)

  //BTX
  // Interaction types - used internally.
  enum
  {
    NONE, PANNING, ZOOMING, SELECTING
  };
  //ETX

protected:
  vtkInteractorStyleAnnotate2D();
  ~vtkInteractorStyleAnnotate2D();

  // The interaction mode
  int Interaction;

  // Draws the selection rubber band
  void RedrawRubberBand();

  // Interpolate lines between points in the selection rubber band.
  void BresenhamLineInterpolate(int x0, int y0, int x1, int y1,
      vtkUnsignedIntArray* outPoints, int pitch);

  // The pixel array for the rubber band
  vtkUnsignedCharArray* PixelArray;

  // Points for the selection
  vtkUnsignedIntArray* SelectionPoints;

  // Whether to render when the mouse moves
  bool RenderOnMouseMove;

private:
  vtkInteractorStyleAnnotate2D(const vtkInteractorStyleAnnotate2D&); // Not implemented
  void operator=(const vtkInteractorStyleAnnotate2D&); // Not implemented
};

#endif // __vtkInteractorStyleAnnotate2D_h
