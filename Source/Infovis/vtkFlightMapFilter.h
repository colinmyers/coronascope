/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkFlightMapFilter.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkFlightMapFilter - Add weights to edges of a graph based on various
// features.
//
// .SECTION Description
// vtkFlightMapFilter adds weights to the edges of a graph based on various
// factors such as edge-length, combined vertex degree of an edge, and
// proximity of an edge to a landmark or annotation. The latter is provided by
// vtkAnnotationLayers as a second input to this filter. The output is a
// spanning tree of the weighted graph, with a choice between the maximum or
// minimum weight span.
//
// The filter is used in CoronaScope to calculate a 'flight path' along which to
// move the camera in automated pan and zoom. Two presets modes, 'express' and
// 'tourist' are provided.
//
// .SEE ALSO
//

#ifndef __vtkFlightMapFilter_h
#define __vtkFlightMapFilter_h

#include "vtkcsmInfovisWin32Header.h"
#include "vtkUndirectedGraphAlgorithm.h"
#include "vtkSmartPointer.h" // for class ivars

class vtkExtractSelectedGraph;
class vtkBoostKruskalMinimumSpanningTree;


class VTK_CSM_INFOVIS_EXPORT vtkFlightMapFilter:
  public vtkUndirectedGraphAlgorithm
{
public:
  static vtkFlightMapFilter *New();
  vtkTypeMacro(vtkFlightMapFilter, vtkUndirectedGraphAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // When set, the filter returns a minimum spanning tree of the weighted graph.
  // Otherwise it is a maximum spanning tree.
  vtkSetMacro(MinimumSpanningTree, bool)
  vtkGetMacro(MinimumSpanningTree, bool)
  vtkBooleanMacro(MinimumSpanningTree, bool)

  // Description:
  // Amount to weight the influence of edge lengths.
  vtkSetClampMacro(EdgeLengthFactor, double, 0.0, 1.0)
  vtkGetMacro(EdgeLengthFactor, double)

  // Description:
  // Amount to weight the influence of edge degrees (i.e. the combined degree of
  // each endpoint).
  vtkSetClampMacro(EdgeDegreeFactor, double, 0.0, 1.0)
  vtkGetMacro(EdgeDegreeFactor, double)

  // Description:
  // Amount to weight the influence of incidence to landmarks vertices.
  vtkSetClampMacro(EdgeLandmarkProximityFactor, double, 0.0, 1.0)
  vtkGetMacro(EdgeLandmarkProximityFactor, double)

  // Description:
  // Presets. Provides settings for each of the 'factors'.
  void SetExpressPreset();
  void SetTouristPreset();

protected:
  vtkFlightMapFilter();
  ~vtkFlightMapFilter();

  int RequestData(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);

  // Description:
  // Set the input type of the algorithm to vtkGraph and vtkDirectedAcyclicGraph.
  int FillInputPortInformation(int port, vtkInformation *info);

  // Description:
  // Set the output type of the algorithm to vtkGraph
  int FillOutputPortInformation(int port, vtkInformation *info);

private:
  vtkFlightMapFilter(const vtkFlightMapFilter&);  // Not implemented.
  void operator=(const vtkFlightMapFilter&);  // Not implemented.

  // The default, initial edge-weight.
  static const double DefaultWeight;
  bool MinimumSpanningTree;
  double EdgeLengthFactor;
  double EdgeDegreeFactor;
  double EdgeLandmarkProximityFactor;

  vtkSmartPointer<vtkBoostKruskalMinimumSpanningTree> Kruskal;
  vtkSmartPointer<vtkExtractSelectedGraph> ExtractKruskalSelection;
};

const double vtkFlightMapFilter::DefaultWeight = 1500.0;

#endif // __vtkFlightMapFilter_h
