/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkMySpanTreeLayoutStrategy.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
//-------------------------------------------------------------------------
//Copyright 2008 Sandia Corporation.
//Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//the U.S. Government retains certain rights in this software.
//-------------------------------------------------------------------------
// .NAME vtkMySpanTreeLayoutStrategy
// .SECTION Description
// vtkSpanTreeLayout is a strategy for drawing directed graphs that
// works by first extracting a spanning tree (more accurately, a
// spanning forest), and using this both to position graph vertices
// and to plan the placement of non-tree edges.  The latter are drawn
// with the aid of edge points to produce a tidy drawing.
//
// The approach is best suited to "quasi-trees", graphs where the number
// of edges is of the same order as the number of nodes; it is less well
// suited to denser graphs.  The boolean flag DepthFirstSpanningTree
// determines whether a depth-first or breadth-first strategy is used to
// construct the underlying forest, and the choice of strategy affects
// the output layout significantly.  Informal experiments suggest that
// the breadth-first strategy is better for denser graphs.
//
// Different layouts could also be produced by plugging in alternative
// tree layout strategies.  To work with the method of routing non-tree
// edges, any strategy should draw a tree so that levels are equally
// spaced along the z-axis, precluding for example the use of a radial
// or balloon layout.
//
// vtkSpanTreeLayout is based on an approach to 3D graph layout first
// developed as part of the "tulip" tool by Dr. David Auber at LaBRI,
// U.Bordeaux: see www.tulip-software.org
//
// This implementation departs from the original version in that:
// (a) it is reconstructed to use Titan/VTK data structures;
// (b) it uses a faster method for dealing with non-tree edges,
//     requiring at most two edge points per edge
// (c) allows for plugging in different tree layout methods
// (d) allows selection of two different strategies for building
//     the underlying layout tree, which can yield significantly
//     different results depending on the data.
//
// .SECTION Thanks
// Thanks to David Duke from the University of Leeds for providing this
// implementation.
// Tree layout setter added by Colin Myers, 2011.

#ifndef __vtkMySpanTreeLayoutStrategy_h
#define __vtkMySpanTreeLayoutStrategy_h

#include "vtkcsmInfovisWin32Header.h"
#include "vtkGraphLayoutStrategy.h"

class VTK_CSM_INFOVIS_EXPORT vtkMySpanTreeLayoutStrategy:
  public vtkGraphLayoutStrategy
{
public:
  static vtkMySpanTreeLayoutStrategy *New();

  vtkTypeMacro(vtkMySpanTreeLayoutStrategy, vtkGraphLayoutStrategy)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // If set, base the layout on a depth-first spanning tree,
  // rather than the default breadth-first spanning tree.
  // Switching between DFT and BFT may significantly change
  // the layout, and choice must be made on a per-graph basis.
  // Default value is off.
  vtkSetMacro(DepthFirstSpanningTree, bool)
  vtkGetMacro(DepthFirstSpanningTree, bool)
  vtkBooleanMacro(DepthFirstSpanningTree, bool)

  // Description:
  // Perform the layout.
  void Layout();

  // Description:
  // Set a tree layout strategy. By default this is set to
  // vtkConeLayoutStrategy
  void SetTreeLayout(vtkGraphLayoutStrategy* treeLayout);
  vtkGetObjectMacro(TreeLayout, vtkGraphLayoutStrategy)

protected:
  vtkMySpanTreeLayoutStrategy();
  ~vtkMySpanTreeLayoutStrategy();

  vtkGraphLayoutStrategy *TreeLayout;
  bool DepthFirstSpanningTree;

private:
  vtkMySpanTreeLayoutStrategy(const vtkMySpanTreeLayoutStrategy&); // Not implemented.
  void operator=(const vtkMySpanTreeLayoutStrategy&);  // Not implemented.
};

#endif
