/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOGDFLayoutStrategy.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOGDFLayoutStrategy - Compute a graph layout using the Open Graph
// Drawing Framework.
//
// .SECTION Description
// Compute a graph layout using the Open Graph Drawing Framework. Several
// layout algorithms can be set by name, currently available are:
// Balloon Circular DavidsonHarel Dominance FMMM GEM MMMExampleFast
// MMMExampleNice MMMExampleNoTwist FruchtermanReingold KamadaKawai
// Sugiyama.
//
// If you want to access individual layout parameters, or use an
// algorithm not in the list above, you can set your own ogdf::LayoutModule.
//
// This class requires libOGDF, see http://www.ogdf.net/doku.php/start for
// download and installation instructions.
//
// .SEE ALSO

#ifndef __vtkOGDFLayoutStrategy_h
#define __vtkOGDFLayoutStrategy_h

#include "vtkcsmInfovisWin32Header.h"
#include "vtkGraphLayoutStrategy.h"

namespace ogdf
{
  class LayoutModule;
}


class VTK_CSM_INFOVIS_EXPORT vtkOGDFLayoutStrategy:
  public vtkGraphLayoutStrategy
{
public:
  static vtkOGDFLayoutStrategy* New();

  vtkTypeMacro(vtkOGDFLayoutStrategy, vtkGraphLayoutStrategy)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Perform the layout.
  void Layout();

  // Description:
  // Select an OGDF Layout Module by name.
  void SetLayoutModuleByName(const char* name);

  // Description:
  // Set the OGDF Layout Module directly. This class takes ownership of 'layout'.
  void SetLayoutModule(ogdf::LayoutModule* layout);

  // Description:
  // Get the OGDF Layout Module.
  ogdf::LayoutModule* GetLayoutModule() const;

  // Description:
  // Use OGDF edge layout. If set on and the OGDF layout provides them, edge
  // points are added to the output graph. Off by default.
  vtkSetMacro(LayoutEdges, bool)
  vtkGetMacro(LayoutEdges, bool)
  vtkBooleanMacro(LayoutEdges, bool)

protected:
  vtkOGDFLayoutStrategy();
  ~vtkOGDFLayoutStrategy();

private:
  vtkOGDFLayoutStrategy(const vtkOGDFLayoutStrategy&);  // Not implemented.
  void operator=(const vtkOGDFLayoutStrategy&);  // Not implemented.

  ogdf::LayoutModule* LayoutModule;
  bool LayoutEdges;
};

#endif
