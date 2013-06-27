/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkOffScreenRepresentation.cxx

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkOffScreenRepresentation.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellCenters.h"
#include "vtkInteractorObserver.h"
#include "vtkObjectFactory.h"
#include "vtkOffScreenRepresentationImpl.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyLine.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkUnsignedCharArray.h"
#include "vtkWindow.h"

//----------------------------------------------------------------------
vtkStandardNewMacro(vtkOffScreenRepresentation)
vtkCxxSetObjectMacro(vtkOffScreenRepresentation, TextProperty, vtkTextProperty)
vtkCxxSetObjectMacro(vtkOffScreenRepresentation, FrameProperty, vtkProperty2D)
vtkCxxSetObjectMacro(vtkOffScreenRepresentation, HighlightPolyData, vtkPolyData)

//----------------------------------------------------------------------
vtkOffScreenRepresentation::vtkOffScreenRepresentation()
{
  this->TextVisible = 0;
  this->HighlightVisible = 0;
  this->FlightPathVisible = 0;
  this->BezelVisible = 1;
  this->ProxiesVisible = 1;
  this->BezelOpacity = 0.25;
  this->HoverText = 0;

  // Controlling layout
  this->Padding = 5;
  this->Offset[0] = 15;
  this->Offset[1] = -30;

  // The text actor
  this->TextMapper = vtkSmartPointer<vtkTextMapper>::New();
  this->TextActor = vtkSmartPointer<vtkActor2D>::New();
  this->TextActor->SetMapper(this->TextMapper);
  this->TextProperty = vtkTextProperty::New();
  this->TextProperty->SetColor(0, 0, 0);
  this->TextProperty->SetFontSize(14);
  this->TextProperty->BoldOn();
  this->TextMapper->SetTextProperty(this->TextProperty);

  // The frame
  this->FramePoints = vtkSmartPointer<vtkPoints>::New();
  this->FramePoints->SetNumberOfPoints(4);
  this->FramePolygon = vtkSmartPointer<vtkCellArray>::New();
  this->FramePolygon->Allocate(this->FramePolygon->EstimateSize(1, 5));
  this->FramePolygon->InsertNextCell(4);
  this->FramePolygon->InsertCellPoint(0);
  this->FramePolygon->InsertCellPoint(1);
  this->FramePolygon->InsertCellPoint(2);
  this->FramePolygon->InsertCellPoint(3);
  this->FramePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->FramePolyData->SetPoints(this->FramePoints);
  this->FramePolyData->SetPolys(this->FramePolygon);
  this->FrameMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->FrameMapper->SetInput(this->FramePolyData);
  this->FrameActor = vtkSmartPointer<vtkActor2D>::New();
  this->FrameActor->SetMapper(this->FrameMapper);
  this->FrameProperty = vtkProperty2D::New();
  this->FrameProperty->SetColor(1.0, 1.0, 0.882);
  this->FrameProperty->SetOpacity(0.5);
  this->FrameActor->SetProperty(this->FrameProperty);

  // The highlight
  this->HighlightPolyData = 0;
  this->HighlightMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->HighlightActor = vtkSmartPointer<vtkActor2D>::New();
  this->HighlightActor->SetMapper(this->HighlightMapper);
  this->HighlightActor->SetLayerNumber(3);
  this->HighlightActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
  this->HighlightActor->GetProperty()->SetOpacity(0.8);

  // The flight path
  this->FlightPathPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->FlightPathMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->FlightPathActor = vtkSmartPointer<vtkActor>::New();
  this->FlightPathMapper->SetScalarVisibility(false);
  this->FlightPathActor->SetMapper(this->FlightPathMapper);
  this->FlightPathActor->GetProperty()->SetOpacity(0.6);
  this->FlightPathActor->GetProperty()->SetColor(1.0, 0.0, 0.2);
  this->FlightPathActor->GetProperty()->SetLineWidth(15.0);
  this->FlightPathActor->SetPosition(0.0, 0.0, 0.003);
  this->FlightPathActor->SetPickable(false);

  // The offscreen proxies
  this->LandmarkCentres = 0;
  this->ProjectOffScreen =
      vtkSmartPointer<vtkOffScreenRepresentationImpl>::New();
  this->ProxyMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ProxyActor = vtkSmartPointer<vtkActor2D>::New();
  this->BezelMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->BezelActor = vtkSmartPointer<vtkActor2D>::New();
  this->EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  this->ProjectOffScreen->SetInput(this->EmptyPolyData);
  this->BezelMapper->SetInputConnection(
      this->ProjectOffScreen->GetOutputPort(1));
  this->ProxyMapper->SetInputConnection(this->ProjectOffScreen->GetOutputPort(0));
  this->ProxyActor->SetMapper(this->ProxyMapper);
  this->BezelActor->SetMapper(this->BezelMapper);

  this->ProxyMapper->ColorByArrayComponent(const_cast<char*>("Hull color"), -1);
  this->ProxyMapper->SetScalarModeToUseCellFieldData();
  this->ProxyMapper->SetScalarVisibility(true);
  this->ProxyActor->SetLayerNumber(0);
  this->ProxyActor->SetPickable(true);

  this->BezelMapper->SetScalarVisibility(false);
  this->BezelActor->SetLayerNumber(1);
  this->BezelActor->GetProperty()->SetOpacity(this->BezelOpacity);
  this->BezelActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
  this->BezelActor->SetPickable(true);
}

//----------------------------------------------------------------------
vtkOffScreenRepresentation::~vtkOffScreenRepresentation()
{
  if (this->HoverText)
  {
    delete[] this->HoverText;
  }

  this->SetFrameProperty(0);
  this->SetTextProperty(0);

  this->SetHighlightPolyData(0);
  this->SetLandmarkCentres(0);
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetLandmarkCentres(
    vtkCellCenters* landmarkCentres)
{
  vtkDebugMacro(
      << this->GetClassName() << " (" << this <<
      "): setting LandmarkCenters to " << landmarkCentres);
  if (this->LandmarkCentres != landmarkCentres)
  {
    vtkCellCenters* temp = this->LandmarkCentres;
    this->LandmarkCentres = landmarkCentres;
    if (this->LandmarkCentres != NULL)
    {
      this->LandmarkCentres->Register(this);
      this->ProjectOffScreen->SetInputConnection(
          this->LandmarkCentres->GetOutputPort());
    }
    if (temp != NULL)
    {
      temp->UnRegister(this);
    }
    this->Modified();
  }
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetWorldSize(double s)
{
  this->ProjectOffScreen->SetWorldSize(s);
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::StartWidgetInteraction(double e[2])
{
  this->StartEventPosition[0] = e[0];
  this->StartEventPosition[1] = e[1];
  this->TextVisible = 1;
  this->HighlightVisible = 1;
  this->BezelVisible = 1;
  this->ProxiesVisible = 1;
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::EndWidgetInteraction(double vtkNotUsed(e)[2])
{
  this->TextVisible = 0;
  this->HighlightVisible = 0;
  this->BezelVisible = 1;
  this->ProxiesVisible = 1;
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::RestoreDim()
{
  this->BezelActor->GetProperty()->SetOpacity(this->BezelOpacity);
  // Set the LandmarkCenters to modified to restore the proxy opacity
  this->LandmarkCentres->Modified();
  this->Modified();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::DimSubtract(double amount)
{
  this->BezelActor->GetProperty()->SetOpacity(
      this->BezelActor->GetProperty()->GetOpacity() - amount);
  this->ProxyActor->GetProperty()->SetOpacity(
      this->BezelActor->GetProperty()->GetOpacity() - amount);
  this->Modified();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::DimMultiply(double amount)
{
  this->BezelActor->GetProperty()->SetOpacity(
      this->BezelActor->GetProperty()->GetOpacity() * amount);

  vtkUnsignedCharArray* colors = vtkUnsignedCharArray::SafeDownCast(
      this->ProjectOffScreen->GetOutput()->GetCellData()->GetAbstractArray(
          "Hull color"));

  if (colors)
  {
    unsigned char oldColour[4];
    for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); ++i)
    {
      colors->GetTupleValue(i, oldColour);
      oldColour[3] *= amount;
      colors->SetTupleValue(i, oldColour);
    }
  }
  this->Modified();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::BuildRepresentation()
{
  if (this->GetMTime() > this->BuildTime
      || (this->Renderer && this->Renderer->GetVTKWindow()
          && this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime))
  {
    this->TextVisible = 0;
    this->HighlightVisible = 0;

    int size[2];
    size[0] = (this->Renderer->GetSize())[0];
    size[1] = (this->Renderer->GetSize())[1];
    int stringSize[2];
    stringSize[0] = stringSize[1] = 0;
    double imageSize[2];
    imageSize[0] = imageSize[1] = 0.0;
    double frameSize[2];
    frameSize[0] = frameSize[1] = 0.0;
    double so[2];
    double fo[2];
    so[0] = 0.0;
    so[1] = 0.0;
    fo[0] = 0.0;
    fo[1] = 0.0;
    double e[2];
    e[0] = static_cast<double>(this->StartEventPosition[0] + this->Offset[0]);
    e[1] = static_cast<double>(this->StartEventPosition[1] + this->Offset[1]);

    // Determine the size of the text
    if (this->HoverText)
    {
      // Start by getting the size of the text
      this->TextMapper->SetInput(this->HoverText);
      this->TextMapper->GetSize(this->Renderer, stringSize);
      this->TextVisible = ((stringSize[0] > 0 && stringSize[1] > 0) ? 1 : 0);
    }

    if (this->TextVisible)
    {
      frameSize[0] = static_cast<double>(stringSize[0] + 2 * this->Padding);
      frameSize[1] = static_cast<double>(stringSize[1] + 2 * this->Padding);
      fo[0] = 0.0;
      fo[1] = 0.0;
      so[0] = static_cast<double>(this->Padding);
      so[1] = static_cast<double>(this->Padding);
    }
    // Reposition the origin of the balloon if it's off the renderer
    if (e[0] < 0)
    {
      e[0] = 0.0;
    }
    if (e[1] < 0)
    {
      e[1] = 0.0;
    }
    if ((e[0] + frameSize[0] + imageSize[0]) > size[0])
    {
      e[0] = size[0] - (frameSize[0] + imageSize[0]);
    }
    if ((e[1] + frameSize[1] + imageSize[1]) > size[1])
    {
      e[1] = size[1] - (frameSize[1] + imageSize[1]);
    }

    // Draw the text if visible
    if (this->TextVisible)
    {
      this->FramePoints->SetPoint(0, e[0] + fo[0], e[1] + fo[1], 0.0);
      this->FramePoints->SetPoint(1, e[0] + fo[0] + frameSize[0], e[1] + fo[1],
          0.0);
      this->FramePoints->SetPoint(2, e[0] + fo[0] + frameSize[0],
          e[1] + fo[1] + frameSize[1], 0.0);
      this->FramePoints->SetPoint(3, e[0] + fo[0], e[1] + fo[1] + frameSize[1],
          0.0);

      this->TextActor->SetPosition(e[0] + so[0], e[1] + so[1]);
    }

    this->FrameActor->SetProperty(this->FrameProperty);
    this->TextMapper->SetTextProperty(this->TextProperty);

    this->HighlightVisible = this->TextVisible;
    if (this->HighlightVisible)
    {
      this->HighlightMapper->SetInput(this->HighlightPolyData);
    }

    this->ProjectOffScreen->SetRenderer(this->Renderer);

    this->BuildTime.Modified();
  }
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->TextActor->ReleaseGraphicsResources(w);
  this->FrameActor->ReleaseGraphicsResources(w);
  this->HighlightActor->ReleaseGraphicsResources(w);
  this->FlightPathActor->ReleaseGraphicsResources(w);

  this->ProxyActor->ReleaseGraphicsResources(w);
  this->BezelActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int vtkOffScreenRepresentation::RenderOverlay(vtkViewport *v)
{
  int count = 0;
  this->BuildRepresentation();

  if (this->TextVisible)
  {
    count += this->FrameActor->RenderOverlay(v);
    count += this->TextActor->RenderOverlay(v);
  }

  if (this->HighlightVisible)
  {
    count += this->HighlightActor->RenderOverlay(v);
  }

  if (this->BezelVisible)
  {
    count += this->BezelActor->RenderOverlay(v);
  }

  if (this->ProxiesVisible)
  {
    count += this->ProxyActor->RenderOverlay(v);
  }

  return count;
}

//----------------------------------------------------------------------
int vtkOffScreenRepresentation::RenderTranslucentPolygonalGeometry(
    vtkViewport* v)
{
  int count = 0;
  if (this->FlightPathVisible)
  {
    count += this->FlightPathActor->RenderTranslucentPolygonalGeometry(v);
  }
  return count;
}

//----------------------------------------------------------------------
int vtkOffScreenRepresentation::HasTranslucentPolygonalGeometry()
{
  return 1;
}

//---------------------------------------------------------------------------
void vtkOffScreenRepresentation::UnHighlightFlightPath()
{
  this->FlightPathVisible = 0;
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::HighlightFlightPath(vtkPoints* itinerary,
    bool connected)
{
  vtkPolyLine* polyLine = vtkPolyLine::New();
  vtkIdType* pts = new vtkIdType[itinerary->GetNumberOfPoints()];
  for (vtkIdType i = 0; i < itinerary->GetNumberOfPoints(); ++i)
  {
    pts[i] = i;
  }
  polyLine->Initialize(itinerary->GetNumberOfPoints(), pts, itinerary);
  delete[] pts;

  vtkCellArray* lines = vtkCellArray::New();
  lines->InsertNextCell(polyLine);
  this->FlightPathPolyData->Reset();
  this->FlightPathPolyData->SetLines(lines);
  this->FlightPathPolyData->SetPoints(itinerary);
  if (connected)
  {
    this->FlightPathActor->GetProperty()->SetLineStipplePattern(65535);
    this->FlightPathActor->GetProperty()->SetLineStippleRepeatFactor(1);
  }
  else
  {
    this->FlightPathActor->GetProperty()->SetLineStipplePattern(127);
    this->FlightPathActor->GetProperty()->SetLineStippleRepeatFactor(1);
  }
  this->FlightPathMapper->SetInput(this->FlightPathPolyData);
  this->FlightPathVisible = 1;
  lines->Delete();
  polyLine->Delete();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetBezelColour(double r, double g, double b,
    double a)
{
  this->BezelActor->GetProperty()->SetColor(r, g, b);
  this->BezelActor->GetProperty()->SetOpacity(a);
  this->BezelOpacity = a;
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::GetBezelColour(double* rgba)
{
  this->BezelActor->GetProperty()->GetColor(rgba[0], rgba[1], rgba[2]);
  rgba[3] = this->BezelActor->GetProperty()->GetOpacity();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetProjectOffScreenVisibility(bool b)
{
  if (b)
  {
    this->ProjectOffScreen->SetInputConnection(
        this->LandmarkCentres->GetOutputPort());
    this->BezelMapper->SetInputConnection(
        this->ProjectOffScreen->GetOutputPort(1));
  }
  else
  {
    this->ProjectOffScreen->SetInput(this->EmptyPolyData);
    this->BezelMapper->SetInput(this->EmptyPolyData);
  }
  this->Modified();
}

//----------------------------------------------------------------------
bool vtkOffScreenRepresentation::GetProjectOffScreenVisibility()
{
  return !(this->ProjectOffScreen->GetInput()
      == (vtkDataObject*) this->EmptyPolyData);
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetReduceOverlaps(bool b)
{
  this->ProjectOffScreen->SetReduceOverlaps(b);
}

//----------------------------------------------------------------------
bool vtkOffScreenRepresentation::GetReduceOverlaps()
{
  return this->ProjectOffScreen->GetReduceOverlaps();
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::SetShowError(bool b)
{
  this->ProjectOffScreen->SetShowError(b);
}

//----------------------------------------------------------------------
bool vtkOffScreenRepresentation::GetShowError()
{
  return this->ProjectOffScreen->GetShowError();
}

//----------------------------------------------------------------------
vtkPolyData* vtkOffScreenRepresentation::GetOffScreenPoly()
{
  return this->ProjectOffScreen->GetOutput();
}

//----------------------------------------------------------------------
vtkActor2D* vtkOffScreenRepresentation::GetOffScreenActor()
{
  return this->ProxyActor;
}

//----------------------------------------------------------------------
void vtkOffScreenRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Hover Text: ";
  if (this->HoverText)
  {
    os << this->HoverText << "\n";
  }
  else
  {
    os << "(none)\n";
  }

  os << indent << "Padding: " << this->Padding << "\n";
  os << indent << "Offset: (" << this->Offset[0] << "," << this->Offset[1]
                                                                        << ")\n";

  if (this->FrameProperty)
  {
    os << indent << "Frame Property:\n";
    this->FrameProperty->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "Frame Property: (none)\n";
  }

  if (this->TextProperty)
  {
    os << indent << "Text Property:\n";
    this->TextProperty->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "Text Property: (none)\n";
  }

  if (this->LandmarkCentres)
  {
    os << indent << "Landmark Centres:\n";
    this->LandmarkCentres->PrintSelf(os, indent.GetNextIndent());
  }
}
