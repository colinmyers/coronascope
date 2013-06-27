/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: vtkOffScreenRepresentationImpl.cxx,v $

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/

#include "vtkOffScreenRepresentationImpl.h"

#include "vtkAppendPolyData.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCoordinate.h"
#include "vtkDataSetAttributes.h"
#include "vtkDiskSource.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSectorSource.h"
#include "vtkRenderer.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"

#include <algorithm>


//-----------------------------------------------------------------------------
// Description:
// Parameters and representation of a single proxy.
struct CoronaScopeProxy
{
  vtkIdType InputPointId;
  double WorldPoint[2];
  double Error;
  double DistanceInWorld;
  vtkSmartPointer<vtkPolyData> Representation;

  //---------------------------------------------------------------------------
  // Description:
  // Get the proxy angle in degrees.
  double GetAngleInDegrees() const
  {
    return this->Angle;
  }

  //---------------------------------------------------------------------------
  // Descrption:
  // Set the proxy angle in degrees. Clamped between 0 and 360.
  void SetAngleInDegrees(double degrees)
  {
    while (degrees < 0)
    {
      degrees += 360.0;
    }

    while (degrees >= 360.0)
    {
      degrees -= 360.0;
    }

    this->Angle = degrees;
  }

private:
  double Angle;
};

//-----------------------------------------------------------------------------
// Description:
// This functor can be used by std::sort to sort vectors of proxies by angle.
struct CoronaScopeProxySortFunctor
{
  bool operator()(const CoronaScopeProxy& proxy1,
      const CoronaScopeProxy& proxy2) const
  {
    return proxy1.GetAngleInDegrees() < proxy2.GetAngleInDegrees();
  }
} CoronaScopeProxySort;

//-----------------------------------------------------------------------------
// Description:
// A segment is used to contain any number of overlapping proxies and provides
// methods for calculating the forward (clockwise edge of a segment) and reverse
// (anticlockwise/counterclockwise edge) extents, both before and after expansion
// of the space required to remove overlaps.
struct CoronaScopeSegment
{
  std::vector<CoronaScopeProxy*> proxies;
  double spacing;
  double width;
  CoronaScopeSegment(double spacing, double width) :
    spacing(spacing), width(width)
  {
  }

  // Description:
  // Print out the values of the segment for debugging purposes.
  void PrintDebugInfo(ostream& os) const
  {
    os << "Expanded-forward: " << this->GetExpandedForwardExtent() << " "
        << "Forward: " << this->GetForwardExtent() << " " << "Centre: "
        << this->GetCentre() << " " << "Reverse: " << this->GetReverseExtent()
        << " " << "Expanded-reverse: " << this->GetExpandedReverseExtent()
        << " " << "Expanded-width: " << this->GetExpandedWidth() << endl;

    for (size_t i = 0; i < proxies.size(); ++i)
    {
      cout << proxies[i]->GetAngleInDegrees() << " ";
    }
    cout << endl;
  }

  // Description:
  // The forward (anticlockwise) extent of the segment without expansion.
  double GetForwardExtent() const
  {
    return this->proxies[0]->GetAngleInDegrees() + width / 2.0;
  }

  // Description:
  // The reverse (clockwise) extent of the segment. Can return a negative angle
  // if centre() is close to zero degrees.
  double GetReverseExtent() const
  {
    return this->proxies[this->proxies.size() - 1]->GetAngleInDegrees()
        - width / 2.0;
  }

  // Description:
  // The centre of the segment.
  double GetCentre() const
  {
    // Ensure the left/right distances are interpreted the right way around:
    // without this the centre is ~180 degrees out
    double right = this->GetReverseExtent();
    if (this->GetForwardExtent() - right > this->GetExpandedWidth())
    {
      right -= 360.0;
    }
    return right + (this->GetForwardExtent() - right) / 2.0;
  }

  // Description:
  // The forward (anticlockwise) extent of the segment after expansion. Can
  // return a negative angle if centre() is close to zero degrees.
  double GetExpandedForwardExtent() const
  {
    return this->GetCentre() + this->GetExpandedWidth() / 2.0;
  }

  // Description:
  // The reverse (clockwise) extent of the segment after expansion.
  double GetExpandedReverseExtent() const
  {
    return this->GetCentre() - this->GetExpandedWidth() / 2.0;
  }

  // Description:
  // The total width of the segment after expansion, i.e. the width * number of
  // proxies plus spacing.
  double GetExpandedWidth() const
  {
    return (this->width + this->spacing) * this->proxies.size();
  }
};

vtkStandardNewMacro(vtkOffScreenRepresentationImpl)

//-----------------------------------------------------------------------------
vtkOffScreenRepresentationImpl::vtkOffScreenRepresentationImpl()
{
  this->Renderer = 0;
  this->WorldSize = 0;
  this->InnerBezelRadius = 0.65;
  this->OuterBezelRadius = 0.70;
  this->CentreSpotRadius = 0.03;
  this->Resolution = 16;
  this->ReduceOverlaps = false;
  this->ShowError = false;

  this->SetNumberOfOutputPorts(2);
  this->Coordinate = vtkSmartPointer<vtkCoordinate>::New();
  this->EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  this->BezelSource = vtkSmartPointer<vtkDiskSource>::New();
  this->BezelTransform = vtkSmartPointer<vtkTransform>::New();
  this->BezelTransformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  this->CentreSpotSource = vtkSmartPointer<vtkDiskSource>::New();
  this->BezelAppend = vtkSmartPointer<vtkAppendPolyData>::New();

  this->ProxySource = vtkSmartPointer<vtkSectorSource>::New();
  this->ProxySourceTransform = vtkSmartPointer<vtkTransform>::New();
  this->PointerSource = vtkSmartPointer<vtkSectorSource>::New();
  this->PointerSourceTransform = vtkSmartPointer<vtkTransform>::New();
  this->PointerAppend = vtkSmartPointer<vtkAppendPolyData>::New();

  this->ProxyTransform = vtkSmartPointer<vtkTransform>::New();
  this->ProxyTransformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  this->ProxyAppend = vtkSmartPointer<vtkAppendPolyData>::New();

  this->BezelSource->SetOuterRadius(this->OuterBezelRadius);
  this->BezelSource->SetInnerRadius(this->InnerBezelRadius);
  this->BezelSource->SetRadialResolution(4);
  this->BezelSource->SetCircumferentialResolution(128);
  this->BezelTransformFilter->SetInputConnection(
      this->BezelAppend->GetOutputPort());
  this->BezelTransformFilter->SetTransform(this->BezelTransform);

  this->CentreSpotSource->SetOuterRadius(this->CentreSpotRadius);
  this->CentreSpotSource->SetInnerRadius(0.0);
  this->CentreSpotSource->SetRadialResolution(1);
  this->CentreSpotSource->SetCircumferentialResolution(16);

  this->ProxySource->SetOuterRadius(this->OuterBezelRadius);
  this->ProxySource->SetInnerRadius(this->InnerBezelRadius);
  this->ProxySource->SetStartAngle(0);
  this->ProxySource->SetEndAngle(3);
  this->ProxySource->SetCircumferentialResolution(this->Resolution);
  this->ProxySource->SetRadialResolution(1);
  this->ProxySource->Update();

  this->PointerSource->SetOuterRadius(0.95);
  this->PointerSource->SetInnerRadius(this->OuterBezelRadius);
  this->PointerSource->SetStartAngle(0);
  this->PointerSource->SetEndAngle(0.75);
  this->PointerSource->SetCircumferentialResolution(this->Resolution);
  this->PointerSource->SetRadialResolution(1);
  this->PointerSource->Update();

  this->ProxyTransformFilter->SetInputConnection(
      this->ProxyAppend->GetOutputPort());
  this->ProxyTransformFilter->SetTransform(this->ProxyTransform);
}

//-----------------------------------------------------------------------------
vtkOffScreenRepresentationImpl::~vtkOffScreenRepresentationImpl()
{
  this->SetRenderer(0);
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::SetRenderer(vtkRenderer* renderer)
{
  this->Renderer = renderer;
}

//-----------------------------------------------------------------------------
unsigned long vtkOffScreenRepresentationImpl::GetMTime()
{
  if (this->Renderer)
  {
    return
        this->Renderer->GetMTime() > this->MTime ?
            this->Renderer->GetMTime() : this->MTime;
  }
  else
  {
    return this->MTime;
  }
}

//-----------------------------------------------------------------------------
double* vtkOffScreenRepresentationImpl::DisplayToWorld(const int point[2]) const
{
  this->Coordinate->SetCoordinateSystemToDisplay();
  this->Coordinate->SetValue(point[0], point[1], 0.0);
  return this->Coordinate->GetComputedWorldValue(this->Renderer);
}

int* vtkOffScreenRepresentationImpl::WorldToDisplay(const double point[2]) const
{
  this->Coordinate->SetCoordinateSystemToWorld();
  this->Coordinate->SetValue(point[0], point[1], 0.0);
  return this->Coordinate->GetComputedDisplayValue(this->Renderer);
}

//-----------------------------------------------------------------------------
vtkIdList* vtkOffScreenRepresentationImpl::CalculateOffScreenPoints(
    vtkPoints* points) const
{
  vtkIdList* ids = vtkIdList::New();

  vtkIdType numPoints = points->GetNumberOfPoints();
  if (numPoints == 0)
  {
    return ids;
  }

  // Get display bounds in world coords
  int* displayRightTop = this->Renderer->GetSize();
  if (displayRightTop[0] == 0 || displayRightTop[1] == 0)
  {
    // Every point is off-screen
    ids->SetNumberOfIds(numPoints);
    for (vtkIdType i = 0; i < numPoints; ++i)
    {
      ids->SetId(i, i);
    }
    return ids;
  }

  int displayLeftBottom[2] =
  { 0, 0 };
  double* coord = this->DisplayToWorld(displayLeftBottom);
  double displayBoundsInWorldLeftBottom[2] =
  { coord[0], coord[1] };
  coord = this->DisplayToWorld(displayRightTop);
  double displayBoundsInWorldRightTop[2] =
  { coord[0], coord[1] };
  double point[3];

  for (vtkIdType i = 0; i < numPoints; ++i)
  {
    points->GetPoint(i, point);
    if (point[0] <= displayBoundsInWorldLeftBottom[0] ||
        point[0] >= displayBoundsInWorldRightTop[0] ||
        point[1] <= displayBoundsInWorldLeftBottom[1] ||
        point[1] >= displayBoundsInWorldRightTop[1])
    {
      ids->InsertNextId(i);
    }
  }

  return ids;
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::CreateRepresentation(
    CoronaScopeProxy& proxy, vtkDataSetAttributes* inAttributes) const
{
  //// Disk segment part
  // Create new arrays, create polydata, allocate existing arrays
  vtkDoubleArray* diskWorldPoints = vtkDoubleArray::New();
  diskWorldPoints->SetName("world point");
  diskWorldPoints->SetNumberOfComponents(2);

  vtkIntArray* diskPolyIds = vtkIntArray::New();
  diskPolyIds->SetName("poly id");

  vtkPolyData* diskPolydata = vtkPolyData::New();
  diskPolydata->ShallowCopy(this->ProxySource->GetOutput());

  vtkDataSetAttributes* diskOutAttributes = diskPolydata->GetAttributes(
      vtkDataSet::CELL);
  diskOutAttributes->CopyAllocate(inAttributes);

  vtkIdType numDiskPolys = diskPolydata->GetNumberOfCells();
  diskWorldPoints->SetNumberOfTuples(numDiskPolys);
  diskPolyIds->SetNumberOfValues(numDiskPolys);

  for (vtkIdType cellId = 0; cellId < numDiskPolys; ++cellId)
  {
    diskOutAttributes->CopyData(inAttributes, proxy.InputPointId, cellId);
    diskWorldPoints->SetTuple(cellId, proxy.WorldPoint);
    diskPolyIds->SetValue(cellId, proxy.InputPointId);
  }

  vtkCellData* diskCellData = diskPolydata->GetCellData();
  diskCellData->AddArray(diskWorldPoints);
  diskCellData->AddArray(diskPolyIds);

  // Transform into position
  double diskRotate = proxy.GetAngleInDegrees()
          - ((this->ProxySource->GetEndAngle() - this->ProxySource->GetStartAngle())
              / 2.0);
  this->ProxySourceTransform->Identity();
  this->ProxySourceTransform->RotateZ(diskRotate);

  vtkPoints* diskTransformedPoints = vtkPoints::New();
  this->ProxySourceTransform->TransformPoints(diskPolydata->GetPoints(),
      diskTransformedPoints);
  diskPolydata->SetPoints(diskTransformedPoints);

  diskWorldPoints->Delete();
  diskPolyIds->Delete();
  diskTransformedPoints->Delete();

  //// Pointer part
  // Create new arrays, create polydata, allocate existing arrays
  vtkDoubleArray* pointerWorldPoints = vtkDoubleArray::New();
  pointerWorldPoints->SetName("world point");
  pointerWorldPoints->SetNumberOfComponents(2);

  vtkIntArray* pointerPolyIds = vtkIntArray::New();
  pointerPolyIds->SetName("poly id");

  vtkPolyData* pointerPolydata = vtkPolyData::New();
  pointerPolydata->ShallowCopy(this->PointerSource->GetOutput());

  vtkDataSetAttributes* pointerOutAttributes = pointerPolydata->GetAttributes(
      vtkDataSet::CELL);
  pointerOutAttributes->CopyAllocate(inAttributes);

  vtkIdType numPointerPolys = pointerPolydata->GetNumberOfCells();
  pointerWorldPoints->SetNumberOfTuples(numPointerPolys);
  pointerPolyIds->SetNumberOfValues(numPointerPolys);

  for (vtkIdType cellId = 0; cellId < numPointerPolys; ++cellId)
  {
    pointerOutAttributes->CopyData(inAttributes, proxy.InputPointId, cellId);
    pointerWorldPoints->SetTuple(cellId, proxy.WorldPoint);
    pointerPolyIds->SetValue(cellId, proxy.InputPointId);
  }

  vtkCellData* pointerCellData = pointerPolydata->GetCellData();
  pointerCellData->AddArray(pointerWorldPoints);
  pointerCellData->AddArray(pointerPolyIds);

  // Transform into position
  double pointerRotate = proxy.GetAngleInDegrees()
          - ((this->PointerSource->GetEndAngle()
              - this->PointerSource->GetStartAngle()) / 2.0);

  this->PointerSourceTransform->Identity();
  this->PointerSourceTransform->RotateZ(pointerRotate);
  this->PointerSourceTransform->Translate(this->OuterBezelRadius, 0.0, 0.0);

  // Scale pointer according to off-screen distance
  this->PointerSourceTransform->Scale(proxy.DistanceInWorld / this->WorldSize,
      1.0, 1.0);

  // Rotate to show error resulting from overlap reduction
  if (this->ReduceOverlaps && this->ShowError)
  {
    this->PointerSourceTransform->RotateZ(proxy.Error);
  }
  this->PointerSourceTransform->Translate(-this->OuterBezelRadius, 0.0, 0.0);

  vtkPoints* pointerTransformedPoints = vtkPoints::New();
  this->PointerSourceTransform->TransformPoints(pointerPolydata->GetPoints(),
      pointerTransformedPoints);
  pointerPolydata->SetPoints(pointerTransformedPoints);

  pointerWorldPoints->Delete();
  pointerPolyIds->Delete();
  pointerTransformedPoints->Delete();

  //// Join the parts and add the final representation to the proxy.
  this->PointerAppend->RemoveAllInputs();
  this->PointerAppend->AddInput(diskPolydata);
  this->PointerAppend->AddInput(pointerPolydata);
  this->PointerAppend->Update();
  vtkSmartPointer<vtkPolyData> representation =
      vtkSmartPointer<vtkPolyData>::New();
  representation->ShallowCopy(this->PointerAppend->GetOutput());
  proxy.Representation = representation;

  diskPolydata->Delete();
  pointerPolydata->Delete();
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::OverlapReduction(
    std::vector<CoronaScopeProxy>& proxies) const
{
  //// Fewer than two proxies means there can be no overlaps.
  size_t numberOfProxies = proxies.size();
  if (numberOfProxies < 2)
  {
    return;
  }

  //// Sort proxies anticlockwise
  std::sort(proxies.begin(), proxies.end(), CoronaScopeProxySort);

  //// Calculate 'width' of a proxy, and a 10% gap
  double width = this->ProxySource->GetEndAngle()
          - this->ProxySource->GetStartAngle();
  double spacing = width * 0.1;

  //// Initialize segments: one proxy per segment
  std::vector<CoronaScopeSegment> segments;
  for (size_t i = 0; i < numberOfProxies; ++i)
  {
    CoronaScopeSegment s(spacing, width);
    s.proxies.push_back(&proxies[i]);
    segments.push_back(s);
  }

  //// Merge overlapping segments
  size_t lastCount = 0;
  do
  {
    std::vector<CoronaScopeSegment> merged;
    std::vector<CoronaScopeSegment>::iterator previous = segments.begin();
    std::vector<CoronaScopeSegment>::iterator current = segments.begin() + 1;
    for (; current != segments.end(); ++current)
    {
      if (previous->GetExpandedForwardExtent()
          > current->GetExpandedReverseExtent())
      {
        current->proxies.insert(current->proxies.end(),
            previous->proxies.begin(), previous->proxies.end());
      }
      else
      {
        merged.push_back(*previous);
      }
      previous = current;
    }
    merged.push_back(*previous);


    // CSM_TODO Here lurks a bug.
    //// Check the end-case where the last segment (up to 360 degrees) could
    //// overlap with the first (from 0 degrees). We rely on the fact that
    //// segments->GetExpandedReverseExtent() can return a negative angle.
    current = merged.begin();
    if (current->GetExpandedReverseExtent() < 0.0
        && previous->GetExpandedForwardExtent()
        > current->GetExpandedReverseExtent())
    {
      current->proxies.insert(current->proxies.begin(),
          previous->proxies.begin(), previous->proxies.end());
      merged.pop_back();
    }

    lastCount = segments.size();
    segments = merged;
  } while (segments.size() > 1 && lastCount != segments.size());

  //// Distribute proxies within each segment
  double separation = (spacing + width);
  std::vector<CoronaScopeSegment>::iterator segment = segments.begin();
  for (; segment != segments.end(); ++segment)
  {
    double angle = segment->GetExpandedReverseExtent() + separation / 2.0;
    for (size_t i = 0; i < segment->proxies.size(); ++i)
    {
      // Calculate the error and store with the proxy info
      segment->proxies[i]->Error = segment->proxies[i]->GetAngleInDegrees()
              - angle;
      segment->proxies[i]->SetAngleInDegrees(angle);
      angle += separation;
    }
  }
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::SetInnerBezelRadius(double r)
{
  this->InnerBezelRadius = r;
  this->BezelSource->SetInnerRadius(r);
  this->ProxySource->SetInnerRadius(r);
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkOffScreenRepresentationImpl::GetInnerBezelRadius()
{
  return this->InnerBezelRadius;
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::SetOuterBezelRadius(double r)
{
  this->OuterBezelRadius = r;
  this->BezelSource->SetOuterRadius(r);
  this->ProxySource->SetOuterRadius(r);
  this->Modified();
}

//-----------------------------------------------------------------------------
double vtkOffScreenRepresentationImpl::GetOuterBezelRadius()
{
  return this->OuterBezelRadius;
}

//-----------------------------------------------------------------------------
int vtkOffScreenRepresentationImpl::RequestData(
    vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // Check the renderer has been set
  if (!this->Renderer)
  {
    vtkErrorMacro(<< "No renderer has been set.");
    return 0;
  }

  // Get the input and output
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *outInfo1 = outputVector->GetInformationObject(1);

  vtkPolyData *input = vtkPolyData::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output1 = vtkPolyData::SafeDownCast(
      outInfo1->Get(vtkDataObject::DATA_OBJECT()));

  /* First, add the bezel and centre-spot
   *
   */
  // Get display bounds and convert to world coords
  int* displayRightTop = this->Renderer->GetSize();
  if (displayRightTop[0] == 0 || displayRightTop[1] == 0)
  {
    vtkWarningMacro(<< "Zero size window");
    return 1;
  }
  int displayCentre[2] =
  { displayRightTop[0] / 2, displayRightTop[1] / 2 };
  double* coord = this->DisplayToWorld(displayCentre);
  double displayCentreInWorld[2] =
  { coord[0], coord[1] };

  this->BezelAppend->RemoveAllInputs();
  this->BezelAppend->AddInput(this->BezelSource->GetOutput());
  this->BezelAppend->AddInput(this->CentreSpotSource->GetOutput());

  this->BezelTransform->Identity();
  this->BezelTransform->Translate(displayCentre[0], displayCentre[1], 0.0);
  // Scale to fill shortest side of display
  if (displayCentre[0] < displayCentre[1])
  {
    this->BezelTransform->Scale(displayCentre[0], displayCentre[0], 1.0);
  }
  else
  {
    this->BezelTransform->Scale(displayCentre[1], displayCentre[1], 1.0);
  }
  this->BezelTransformFilter->Update();
  output1->ShallowCopy(this->BezelTransformFilter->GetOutput());

  /* PART A: Find offscreen points
   * IN:
   *    input points of cluster centroids (world)
   *    display bounds in world coords
   * OUT:
   *    offscreen points (display)
   *    distances (world)
   *    directions (unit vector)
   */

  // Get input points/cell centers
  vtkPoints* inPoints = input->GetPoints();
  if (!inPoints)
  {
    return 1;
  }

  // Estimate world size from input if not set explicitly
  if (this->WorldSize <= 0)
  {
    double* bb = input->GetBounds();
    this->WorldSize = sqrt(
        (bb[1] - bb[0] * bb[1] - bb[0]) + (bb[3] - bb[2] * bb[3] - bb[2]));
  }

  // Determine direction and distance to each offscreen point
  std::vector<CoronaScopeProxy> offScreenProxies;
  double inPoint[3];
  double trPoint[2];
  double angle;
  double distance;
  CoronaScopeProxy offScreenProxy;

  // First, get a list of points that are off screen
  vtkIdList* offScreenIds = this->CalculateOffScreenPoints(inPoints);
  vtkIdType numOffScreenPoints = offScreenIds->GetNumberOfIds();

  // Second, build a list of 'proxy' objects
  for (vtkIdType i = 0; i < numOffScreenPoints; ++i)
  {
    vtkIdType j = offScreenIds->GetId(i);
    inPoints->GetPoint(j, inPoint);
    trPoint[0] = inPoint[0] - displayCentreInWorld[0];
    trPoint[1] = inPoint[1] - displayCentreInWorld[1];

    distance = sqrt(trPoint[0] * trPoint[0] + trPoint[1] * trPoint[1]);
    angle = atan2(trPoint[1], trPoint[0]);

    offScreenProxy.DistanceInWorld = distance;
    offScreenProxy.SetAngleInDegrees(vtkMath::DegreesFromRadians(angle));
    offScreenProxy.InputPointId = j;
    offScreenProxy.WorldPoint[0] = inPoint[0];
    offScreenProxy.WorldPoint[1] = inPoint[1];
    offScreenProxy.Error = 0.0;
    offScreenProxies.push_back(offScreenProxy);
  }
  offScreenIds->Delete();

  /* PART B: Overlap reduction
   * IN:
   *   list of offscreen proxy objects
   * OUT:
   *   list of offscreen proxies with angles adjusted
   *   to avoid overlaps
   */
  if (this->ReduceOverlaps)
  {
    this->OverlapReduction(offScreenProxies);
  }

  /* PART C: Glyph each offscreen point
   * IN:
   *    directions to offscreen points (unit vector)
   *    distance to offscreen points (world coordinates)
   *    attributes of related input points
   * OUT:
   *    list of proxy representations
   */

  // Get attributes from input
  vtkDataSetAttributes* inAttributes = input->GetAttributes(vtkDataSet::POINT);
  size_t numOffScreenProxies = offScreenProxies.size();

  // Glyph each point
  for (size_t i = 0; i < numOffScreenProxies; ++i)
  {
    this->CreateRepresentation(offScreenProxies.at(i), inAttributes);
  }

  /* PART D: Append the proxy representation polydatas into the output
   *
   */
  this->ProxyAppend->RemoveAllInputs();
  if (numOffScreenProxies == 0)
  {
    this->ProxyAppend->AddInput(this->EmptyPolyData);
    this->ProxyAppend->Update();
    output->ShallowCopy(this->ProxyAppend->GetOutput());
  }
  else
  {
    for (size_t i = 0; i < numOffScreenProxies; ++i)
    {
      this->ProxyAppend->AddInput(offScreenProxies.at(i).Representation);
    }
    this->ProxyTransform->Identity();
    this->ProxyTransform->Translate(displayCentre[0], displayCentre[1], 0.0);
    if (displayCentre[0] < displayCentre[1])
    {
      this->ProxyTransform->Scale(displayCentre[0], displayCentre[0], 1.0);
      this->ProxyTransformFilter->Update();
      output->ShallowCopy(this->ProxyTransformFilter->GetOutput());
    }
    else
    {
      this->ProxyTransform->Scale(displayCentre[1], displayCentre[1], 1.0);
      this->ProxyTransformFilter->Update();
      output->ShallowCopy(this->ProxyTransformFilter->GetOutput());
    }
  }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkOffScreenRepresentationImpl::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Renderer: " << this->Renderer;
  if (this->Renderer)
  {
    this->Renderer->PrintSelf(os, indent);
  }
  os << indent << "WorldSize: " << this->WorldSize << endl;
  os << indent << "InnerBezelRadius: " << this->InnerBezelRadius << endl;
  os << indent << "OuterBezelRadius: " << this->OuterBezelRadius << endl;
  os << indent << "CentreSpotRadius: " << this->CentreSpotRadius << endl;
  os << indent << "Resolution: " << this->Resolution << endl;
  os << indent << "ReduceOverlaps: " << (this->ReduceOverlaps ? "On" : "Off")
          << endl;
  os << indent << "ShowError: " << (this->ShowError ? "On" : "Off") << endl;
}
