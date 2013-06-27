/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkInteractorStyleAnnotate2D.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkInteractorStyleAnnotate2D.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"

vtkStandardNewMacro(vtkInteractorStyleAnnotate2D)

//--------------------------------------------------------------------------
vtkInteractorStyleAnnotate2D::vtkInteractorStyleAnnotate2D()
{
  this->PixelArray = vtkUnsignedCharArray::New();
  this->SelectionPoints = vtkUnsignedIntArray::New();
  this->SelectionPoints->SetNumberOfComponents(2);
  this->Interaction = NONE;
  this->RenderOnMouseMove = false;
}

//--------------------------------------------------------------------------
vtkInteractorStyleAnnotate2D::~vtkInteractorStyleAnnotate2D()
{
  this->PixelArray->Delete();
  this->SelectionPoints->Delete();
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnKeyPress()
{
  if (strcmp(this->Interactor->GetKeySym(), "Delete") == 0)
  {
    this->InvokeEvent(vtkCommand::UserEvent);
  }
  Superclass::OnKeyPress();
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnLeftButtonDown()
{
  if (this->Interaction == NONE)
  {
    this->Interaction = SELECTING;
    vtkRenderWindow *renWin = this->Interactor->GetRenderWindow();
    this->SelectionPoints->Initialize();
    this->SelectionPoints->InsertNextTupleValue(
        reinterpret_cast<unsigned int*>(this->Interactor->GetEventPosition()));

    this->PixelArray->Initialize();
    this->PixelArray->SetNumberOfComponents(4);
    int *size = renWin->GetSize();
    this->PixelArray->SetNumberOfTuples(size[0] * size[1]);
    renWin->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1,
        this->PixelArray);

    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
        this->Interactor->GetEventPosition()[1]);
    this->InvokeEvent(vtkCommand::StartInteractionEvent);

  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnLeftButtonUp()
{
  if (this->Interaction == SELECTING)
  {
    this->Interaction = NONE;

    // Clear the rubber band
    int* size = this->Interactor->GetRenderWindow()->GetSize();
    unsigned char* pixels = this->PixelArray->GetPointer(0);
    this->Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0] - 1,
        size[1] - 1, pixels, 0);
    this->Interactor->GetRenderWindow()->Frame();
    this->InvokeEvent(vtkCommand::AnnotationChangedEvent,
        reinterpret_cast<void*>(this->SelectionPoints));
    this->InvokeEvent(vtkCommand::EndInteractionEvent);
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnMiddleButtonDown()
{
  if (this->Interaction == NONE)
  {
    this->Interaction = PANNING;
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
        this->Interactor->GetEventPosition()[1]);
    this->InvokeEvent(vtkCommand::StartInteractionEvent);
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnMiddleButtonUp()
{
  if (this->Interaction == PANNING)
  {
    this->Interaction = NONE;
    this->InvokeEvent(vtkCommand::EndInteractionEvent);
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnRightButtonDown()
{
  if (this->Interaction == NONE)
  {
    this->Interaction = ZOOMING;
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
        this->Interactor->GetEventPosition()[1]);
    this->InvokeEvent(vtkCommand::StartInteractionEvent);
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnRightButtonUp()
{
  if (this->Interaction == ZOOMING)
  {
    this->Interaction = NONE;
    this->InvokeEvent(vtkCommand::EndInteractionEvent);
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnMouseMove()
{
  if (this->Interaction == PANNING || this->Interaction == ZOOMING)
  {
    vtkRenderWindowInteractor* rwi = this->GetInteractor();
    int lastPt[] =
    { 0, 0 };
    rwi->GetLastEventPosition(lastPt);
    int curPt[] =
    { 0, 0 };
    rwi->GetEventPosition(curPt);

    vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
    double lastScale = 2.0 * camera->GetParallelScale()
            / this->CurrentRenderer->GetSize()[1];
    double lastFocalPt[] =
    { 0, 0, 0 };
    camera->GetFocalPoint(lastFocalPt);
    double lastPos[] =
    { 0, 0, 0 };
    camera->GetPosition(lastPos);

    if (this->Interaction == PANNING)
    {
      double delta[] =
      { 0, 0, 0 };
      delta[0] = -lastScale * (curPt[0] - lastPt[0]);
      delta[1] = -lastScale * (curPt[1] - lastPt[1]);
      delta[2] = 0;
      camera->SetFocalPoint(lastFocalPt[0] + delta[0],
          lastFocalPt[1] + delta[1], lastFocalPt[2] + delta[2]);
      camera->SetPosition(lastPos[0] + delta[0], lastPos[1] + delta[1],
          lastPos[2] + delta[2]);
      this->InvokeEvent(vtkCommand::InteractionEvent);
      rwi->Render();
    }
    else if (this->Interaction == ZOOMING)
    {
      double motion = 10.0;
      double dyf = motion * (curPt[1] - lastPt[1])
              / this->CurrentRenderer->GetCenter()[1];
      double factor = pow(1.1, dyf);
      camera->SetParallelScale(camera->GetParallelScale() / factor);
      this->InvokeEvent(vtkCommand::InteractionEvent);
      rwi->Render();
    }
  }
  else if (this->Interaction == SELECTING)
  {
    int* endPosition = this->Interactor->GetEventPosition();
    int* size = this->Interactor->GetRenderWindow()->GetSize();
    if (endPosition[0] > (size[0] - 1))
    {
      endPosition[0] = size[0] - 1;
    }
    if (endPosition[0] < 0)
    {
      endPosition[0] = 0;
    }
    if (endPosition[1] > (size[1] - 1))
    {
      endPosition[1] = size[1] - 1;
    }
    if (endPosition[1] < 0)
    {
      endPosition[1] = 0;
    }
    this->SelectionPoints->InsertNextTupleValue(
        reinterpret_cast<unsigned int*>(endPosition));
    this->InvokeEvent(vtkCommand::InteractionEvent);
    this->RedrawRubberBand();
  }
  else if (this->RenderOnMouseMove)
  {
    this->GetInteractor()->Render();
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnMouseWheelForward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
      this->Interactor->GetEventPosition()[1]);
  vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }
  this->Interaction = ZOOMING;
  double motion = 10.0;
  double dyf = motion * 0.2;
  double factor = pow(1.1, dyf);
  camera->SetParallelScale(camera->GetParallelScale() / factor);
  this->InvokeEvent(vtkCommand::InteractionEvent);
  this->GetInteractor()->Render();
  this->Interaction = NONE;
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
      this->Interactor->GetEventPosition()[1]);
  vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
  if (!camera)
  {
    return;
  }
  this->Interaction = ZOOMING;
  double motion = 10.0;
  double dyf = motion * -0.2;
  double factor = pow(1.1, dyf);
  camera->SetParallelScale(camera->GetParallelScale() / factor);
  this->InvokeEvent(vtkCommand::InteractionEvent);
  this->GetInteractor()->Render();
  this->Interaction = NONE;
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::RedrawRubberBand()
{
  // Update the rubber band on the screen
  int *size = this->Interactor->GetRenderWindow()->GetSize();

  vtkUnsignedCharArray *tmpPixelArray = vtkUnsignedCharArray::New();
  tmpPixelArray->DeepCopy(this->PixelArray);
  unsigned char *pixels = tmpPixelArray->GetPointer(0);
  vtkIdType numSelectionPoints = this->SelectionPoints->GetNumberOfTuples();

  // Interpolate selected points
  unsigned int* beginPt = new unsigned int[2];
  unsigned int* endPt = new unsigned int[2];
  vtkUnsignedIntArray* outPoints = vtkUnsignedIntArray::New();

  this->SelectionPoints->GetTupleValue(0, beginPt);
  for (vtkIdType i = 1; i < numSelectionPoints; ++i)
  {
    this->SelectionPoints->GetTupleValue(i, endPt);
    this->BresenhamLineInterpolate(beginPt[0], beginPt[1], endPt[0], endPt[1],
        outPoints, 4 * size[0]);
    beginPt[0] = endPt[0];
    beginPt[1] = endPt[1];
    for (int i = 0; i < outPoints->GetNumberOfTuples(); ++i)
    {
      pixels[outPoints->GetValue(i) + 0] = 255;
      pixels[outPoints->GetValue(i) + 1] = 0;
      pixels[outPoints->GetValue(i) + 2] = 0;
    }
  }

  // Interpolate closure
  this->SelectionPoints->GetTupleValue(numSelectionPoints - 1, beginPt);
  this->SelectionPoints->GetTupleValue(0, endPt);
  this->BresenhamLineInterpolate(beginPt[0], beginPt[1], endPt[0], endPt[1],
      outPoints, 4 * size[0]);
  for (int i = 0; i < outPoints->GetNumberOfTuples(); ++i)
  {
    pixels[outPoints->GetValue(i) + 0] = 0;
    pixels[outPoints->GetValue(i) + 1] = 255;
    pixels[outPoints->GetValue(i) + 2] = 0;
  }

  this->Interactor->GetRenderWindow()->SetRGBACharPixelData(0, 0, size[0] - 1,
      size[1] - 1, pixels, 0);
  this->Interactor->GetRenderWindow()->Frame();

  tmpPixelArray->Delete();
  outPoints->Delete();
  delete[] beginPt;
  delete[] endPt;
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::BresenhamLineInterpolate(int x0, int y0,
    int x1, int y1, vtkUnsignedIntArray* outPoints, int pitch)
{
  outPoints->Initialize();

  if (x0 > x1)
  {
    int tmp = x0;
    x0 = x1;
    x1 = tmp;
    tmp = y0;
    y0 = y1;
    y1 = tmp;
  }

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int ix;
  int iy;

  if (dx >= 0)
  {
    ix = 4;
  }
  else
  {
    ix = -4;
    dx = abs(dx);
  }

  if (dy >= 0)
  {
    iy = pitch;
  }
  else
  {
    iy = -pitch;
    dy = abs(dy);
  }

  if (y0 > y1)
  {
    iy *= -1;
  }

  int dx2 = dx * 2;
  int dy2 = dy * 2;
  int err = 0;
  int nextPx = (4 * x0) + (y0 * pitch);

  if (dx > dy)
  {
    err = dy2 - dx;
    for (int i = 0; i <= dx; ++i)
    {
      outPoints->InsertNextValue(nextPx);
      if (err >= 0)
      {
        err -= dx2;
        nextPx += iy;
      }
      err += dy2;
      nextPx += ix;
    }
  }
  else
  {
    err = dx2 - dy;
    for (int i = 0; i < +dy; ++i)
    {
      outPoints->InsertNextValue(nextPx);
      if (err >= 0)
      {
        err -= dy2;
        nextPx += ix;
      }
      err += dx2;
      nextPx += iy;
    }
  }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleAnnotate2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Interaction: " << this->Interaction << endl;
  os << indent << "RenderOnMouseMove: " <<
      (this->RenderOnMouseMove ? "On" : "Off") << endl;
}
