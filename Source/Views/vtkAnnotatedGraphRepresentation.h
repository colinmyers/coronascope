/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAnnotatedGraphRepresentation.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkAnnotatedGraphRepresentation - Provides a representation for
// vtkAnnotatedGraphView.
//
// .SECTION Description
// vtkAnnotatedGraphView performs graph layout and displays a vtkGraph using
// vtkGraphLayoutView, and additionally displays convex hulls around selections
// of vertices, as defined by the data carried via vtkAnnotationLink. The
// displayed annotations are referred to as 'landmarks'.
//
// .SEE ALSO
// vtkAnnotatedGraphView
//

#ifndef vtkAnnotatedGraphRepresentation_h_
#define vtkAnnotatedGraphRepresentation_h_

#include "vtkcsmViewsWin32Header.h"
#include "vtkRenderedGraphRepresentation.h"
#include "vtkSmartPointer.h" // for SP ivars

class vtkActor;
class vtkCellCenters;
class vtkGraphAnnotationLayersFilter;
class vtkPoints;
class vtkPointSetToLabelHierarchy;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkSelectionNode;
class vtkStringArray;


class vtkAnnotatedGraphRepresentation: public vtkRenderedGraphRepresentation
{
public:
  static vtkAnnotatedGraphRepresentation* New();
  vtkTypeMacro(vtkAnnotatedGraphRepresentation, vtkRenderedGraphRepresentation)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set default colours on graph elements. See vtkApplyColors.
  virtual void SetDefaultVertexColor(double r, double g, double b, double a);
  virtual void GetDefaultVertexColor(double* rgba);
  virtual void SetDefaultEdgeColor(double r, double g, double b, double a);
  virtual void GetDefaultEdgeColor(double* rgba);

  // Description:
  // Get/Set colours of currently selected graph elements. See vtkApplyColors.
  virtual void SetSelectedVertexColour(double r, double g, double b, double a);
  virtual void GetSelectedVertexColour(double* rgba);
  virtual void SetSelectedEdgeColour(double r, double g, double b, double a);
  virtual void GetSelectedEdgeColour(double* rgba);
  virtual void SetUseCurrentAnnotationColour(bool b);
  virtual bool GetUseCurrentAnnotationColour();
  vtkBooleanMacro(UseCurrentAnnotationColour, bool)

  // Description:
  // Add a new annotation.
  virtual void Annotate(vtkView* view, vtkAnnotationLayers* annotations,
      bool extend);

  // Description:
  // Make a selection.
  virtual void Select(vtkView* view, vtkSelection* selection, bool extend);

  // Description:
  // Remove the last annotation that was added.
  void UndoLastAnnotation();

  // Description:
  // Add an annotation from all the graph points found within the 'lasso',
  // described as a set of points.
  void LassoSelect(vtkPoints* lassoPoints, bool);

  // Landmarks (annotations layer)
  virtual void SetScaleFactor(double scale);
  virtual void SetScaleToCamera(bool b);
  virtual void SetMinimumSize(unsigned size);
  virtual void SetOutlineVisibility(bool b);
  virtual void SetLandmarkOutlineColour(double r, double g, double b, double a);
  virtual void GetLandmarkOutlineColour(double* rgba);
  virtual void SetLandmarkToConvexHull();
  virtual void SetLandmarkToRectangle();
  virtual vtkCellCenters* GetLandmarkCentres();

  // Description:
  // Get/Set the array name to use for labelling the landmarks.
  virtual void SetLandmarkLabelArrayName(const char* name);
  virtual const char* GetLandmarkLabelArrayName();

  // Description:
  // Show/hide landmarks.
  virtual void SetLandmarkLabelVisibility(bool b);
  virtual bool GetLandmarkLabelVisibility();
  vtkBooleanMacro(LandmarkLabelVisibility, bool)

  // Description:
  // Get/Set the properties of the landmark label text.
  virtual void SetLandmarkLabelTextProperty(vtkTextProperty* p);
  virtual vtkTextProperty* GetLandmarkLabelTextProperty();

  // Description:
  // Get a list of the arrays available for labelling edges and vertices.
  virtual vtkStringArray* GetEdgeArrayNames();
  virtual vtkStringArray* GetVertexArrayNames();

protected:
  vtkAnnotatedGraphRepresentation();
  ~vtkAnnotatedGraphRepresentation();

  // Description:
  // Called by the view to add/remove this representation.
  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);

  // Description:
  // Handle selection/annotation changes.
  virtual void UpdateAnnotations(vtkAnnotationLayers* annotations, bool extend);
  virtual void UpdateSelection(vtkSelection* selection, bool extend);
  virtual vtkAnnotationLayers* ConvertAnnotations(vtkView* view,
      vtkAnnotationLayers* annotations);
  virtual vtkSelection* ConvertSelection(vtkView* view, vtkSelection* sel);

  // Description:
  // Remember the last selection added for later undoing.
  vtkSelectionNode* LastSelectionNode;

  // Description:
  // Connect inputs to internal pipeline.
  virtual int RequestData(vtkInformation* request,
      vtkInformationVector** inputVector, vtkInformationVector* outputVector);

  vtkSmartPointer<vtkGraphAnnotationLayersFilter> LandmarkGlyph;
  vtkSmartPointer<vtkActor> LandmarkActor;
  vtkSmartPointer<vtkPolyDataMapper> LandmarkMapper;
  vtkSmartPointer<vtkActor> LandmarkOutlineActor;
  vtkSmartPointer<vtkPolyDataMapper> LandmarkOutlineMapper;
  vtkSmartPointer<vtkCellCenters> LandmarkCentres;
  vtkSmartPointer<vtkPointSetToLabelHierarchy> LandmarkLabelHierarchy;

private:
  vtkAnnotatedGraphRepresentation(const vtkAnnotatedGraphRepresentation&); // Not implemented
  void operator=(const vtkAnnotatedGraphRepresentation&);   // Not implemented
};

#endif /* vtkAnnotatedGraphRepresentation_h_ */
