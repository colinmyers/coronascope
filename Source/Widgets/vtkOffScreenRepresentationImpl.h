/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkOffScreenRepresentationImpl.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkOffScreenRepresentationImpl - Represent proxies of off-screen input
// points.
//
// .SECTION Description
// This class is the core of the "CoronaScope" widget. The input is a vtkPolyData
// that contains a set of points in world coordinates. This class determines
// which of those points is currently off-screen (strictly speaking they are
// outside of the viewport), and creates a <i>proxy</i> representation for each
// off-screen target. A circular <i>bezel</i> is drawn in the centre of the
// scene, and the proxies are placed upon the bezel, on a line drawn from the
// centre of the screen to the off-screen target.
//
// Each proxy has a <i>pointer</i>, an arrow-like feature that grows as the
// distance from the centre of the screen to the target increases during a pan
// operation (zoom does not affect the distance since it is calculated in world
// coordinates). To facilitate this it is necessary to call SetWorldSize().
//
// The design here makes use of opacity to enable the creation of 'visual levels'.
// This leads to visual clutter when proxies overlap so an algorithm to remove
// overlaps is available by setting ReduceOverlapsOn(). This works by adjusting
// the position of the proxies as little as possible, and maintaining the
// original order around the bezel. This introduces an error which can be
// visualized by selecting ShowErrorOn().
//
// .SEE ALSO
// vtkOffScreenRepresentation vtkOffScreenWidget

#ifndef __vtkOffScreenRepresentationImpl_h
#define __vtkOffScreenRepresentationImpl_h

#include "vtkcsmWidgetsWin32Header.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h" // needed for ivars

// Private class used to manage various proxy attributes.
struct CoronaScopeProxy;

class vtkAppendPolyData;
class vtkCoordinate;
class vtkDataSetAttributes;
class vtkDiskSource;
class vtkRenderer;
class vtkSectorSource;
class vtkTransform;
class vtkTransformFilter;


class VTK_CSM_WIDGETS_EXPORT vtkOffScreenRepresentationImpl:
public vtkPolyDataAlgorithm
{
public:
  static vtkOffScreenRepresentationImpl *New();
  vtkTypeMacro(vtkOffScreenRepresentationImpl, vtkPolyDataAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the renderer for determining which points are off-screen.
  void SetRenderer(vtkRenderer* renderer);

  // Description:
  // Synchronize the modified time with the renderer to respond to window changes.
  virtual unsigned long GetMTime();

  // Description:
  // Proportion of the display to place the inner bezel ring (0.0 - 1.0). Default 0.8.
  void SetInnerBezelRadius(double r);
  double GetInnerBezelRadius();

  // Description:
  // Proportion of the display to place the outer bezel ring (0.0 - 1.0). Default 0.9.
  void SetOuterBezelRadius(double r);
  double GetOuterBezelRadius();

  // Description:
  // Set the resolution of the bezel ring. Default 6.
  vtkSetClampMacro(Resolution, unsigned, 1, VTK_UNSIGNED_INT_MAX);
  vtkGetMacro(Resolution, unsigned);

  // Description:
  // Get/set radius of the centre spot.
  void SetCentreSpotRadius(double r);
  double GetCentreSpotRadius();

  // Description:
  // Set on to perturb proxies so that overlap is minimised.
  vtkSetMacro(ReduceOverlaps, bool)
  vtkGetMacro(ReduceOverlaps, bool)
  vtkBooleanMacro(ReduceOverlaps, bool)

  // Description:
  // Depict the error resulting from overlap reduction.
  vtkSetMacro(ShowError, bool)
  vtkGetMacro(ShowError, bool)
  vtkBooleanMacro(ShowError, bool)

  // Description:
  // Explicitly set the world size used to calculate pointer lengths. If 0
  // (the default), the size is calculated from the bounding box of the input.
  vtkSetMacro(WorldSize, double)
  vtkGetMacro(WorldSize, double)

protected:
  vtkOffScreenRepresentationImpl();
  ~vtkOffScreenRepresentationImpl();

  // Description:
  // Create polydata containing off-screen proxies and bezel/centre spot.
  int RequestData(vtkInformation*, vtkInformationVector**,
      vtkInformationVector*);

  // Description:
  // Convenience methods for converting between coordinate systems in 2D.
  double* DisplayToWorld(const int point[2]) const;
  int* WorldToDisplay(const double point[2]) const;

  // Description:
  // Calculate which points are off-screen. Returns a list of ids.
  vtkIdList* CalculateOffScreenPoints(vtkPoints* points) const;

  // Description:
  // Build the polydata that make up the proxies.
  void CreateRepresentation(CoronaScopeProxy& proxy,
      vtkDataSetAttributes* inAttributes) const;

  // Description:
  // Remove overlaps between proxies by moving them as little as possible while
  // maintaining the original ordering.
  void OverlapReduction(std::vector<CoronaScopeProxy>& proxies) const;

private:
  vtkOffScreenRepresentationImpl(const vtkOffScreenRepresentationImpl&); // Not implemented.
  void operator=(const vtkOffScreenRepresentationImpl&);  // Not implemented.

  vtkRenderer* Renderer;
  double WorldSize;
  double OuterBezelRadius;
  double InnerBezelRadius;
  double CentreSpotRadius;
  unsigned Resolution;
  bool ReduceOverlaps;
  bool ShowError;

  vtkSmartPointer<vtkCoordinate> Coordinate;
  vtkSmartPointer<vtkPolyData> EmptyPolyData;

  vtkSmartPointer<vtkDiskSource> BezelSource;
  vtkSmartPointer<vtkTransform> BezelTransform;
  vtkSmartPointer<vtkTransformFilter> BezelTransformFilter;
  vtkSmartPointer<vtkDiskSource> CentreSpotSource;
  vtkSmartPointer<vtkAppendPolyData> BezelAppend;

  vtkSmartPointer<vtkSectorSource> ProxySource;
  vtkSmartPointer<vtkTransform> ProxySourceTransform;
  vtkSmartPointer<vtkSectorSource> PointerSource;
  vtkSmartPointer<vtkTransform> PointerSourceTransform;
  vtkSmartPointer<vtkAppendPolyData> PointerAppend;

  vtkSmartPointer<vtkTransform> ProxyTransform;
  vtkSmartPointer<vtkTransformFilter> ProxyTransformFilter;
  vtkSmartPointer<vtkAppendPolyData> ProxyAppend;
};

#endif // __vtkOffScreenRepresentationImpl_h
