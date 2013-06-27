/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkAnnotatedGraphView.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkAnnotatedGraphView - Lays out and displays an annotated graph
//
// .SECTION Description
// vtkAnnotatedGraphView performs graph layout and displays a vtkGraph using
// vtkGraphLayoutView, and additionally displays convex hulls around selections
// of vertices, as defined by the data carried via vtkAnnotationLink.
//
// The default representation is a vtkAnnotatedGraphRepresentation. The default
// interactions are provided by vtkInteractorStyleAnnotated2D. Many more methods
// are provided via the superclass vtkGraphLayoutView.
//
// .SEE ALSO
// vtkAnnotatedGraphRepresentation vtkInteractorStyleAnnotate2D
// vtkGraphAnnotationLayersFilter vtkConvexHull2D

#ifndef __vtkAnnotatedGraphView_h
#define __vtkAnnotatedGraphView_h

#include "vtkcsmViewsWin32Header.h"
#include "vtkGraphLayoutView.h"

class vtkAnnotatedGraphRepresentation;
class vtkAnnotationLayers;
class vtkSelection;


class VTK_CSM_VIEWS_EXPORT vtkAnnotatedGraphView: public vtkGraphLayoutView
{
public:
  static vtkAnnotatedGraphView *New();
  vtkTypeMacro(vtkAnnotatedGraphView, vtkGraphLayoutView)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the representation.
  void SetAnnotatedGraphRepresentation(vtkAnnotatedGraphRepresentation* repr);

protected:
  vtkAnnotatedGraphView();
  ~vtkAnnotatedGraphView();

  // Description:
  // This method is called by Add/SetRepresentationFromInputConnection.
  // See vtkView.
  virtual vtkDataRepresentation* CreateDefaultRepresentation(
      vtkAlgorithmOutput* conn);

  // Description:
  // Get the representation.
  virtual vtkAnnotatedGraphRepresentation* GetAnnotatedGraphRepresentation();

  // Description:
  // Intercept AnnotationChanged, SelectionChanged, and KeyPress events.
  virtual void ProcessEvents(vtkObject* caller, unsigned long eventId,
      void* callData);

  // Description:
  // Generate suitable annotations and selections in reponse to events from the
  // interactor.
  virtual void GenerateAnnotation(void* callData, vtkAnnotationLayers* layers);
  virtual void GenerateSelection(void* callData, vtkSelection* sel);

private:
  vtkAnnotatedGraphView(const vtkAnnotatedGraphView&);  // Not implemented.
  void operator=(const vtkAnnotatedGraphView&);  // Not implemented.
};

#endif /* vtkAnnotatedGraphView_H_ */
