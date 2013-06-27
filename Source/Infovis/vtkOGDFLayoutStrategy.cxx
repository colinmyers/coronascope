#include "vtkOGDFLayoutStrategy.h"

#include "vtkEdgeListIterator.h"
#include "vtkGraph.h"
#include "vtkGraphLayout.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"

#include <ogdf/module/LayoutModule.h>
#include <ogdf/misclayout/BalloonLayout.h>
#include <ogdf/misclayout/CircularLayout.h>
#include <ogdf/energybased/DavidsonHarelLayout.h>
#include <ogdf/upward/DominanceLayout.h>
#include <ogdf/energybased/FMMMLayout.h>
#include <ogdf/energybased/GEMLayout.h>
#include <ogdf/energybased/multilevelmixer/MMMExampleFastLayout.h>
#include <ogdf/energybased/multilevelmixer/MMMExampleNiceLayout.h>
#include <ogdf/energybased/multilevelmixer/MMMExampleNoTwistLayout.h>
#include <ogdf/energybased/SpringEmbedderFR.h>
#include <ogdf/energybased/SpringEmbedderKK.h>
#include <ogdf/layered/SugiyamaLayout.h>

#include <algorithm>
#include <map>
#include <string>


vtkStandardNewMacro(vtkOGDFLayoutStrategy)

//-----------------------------------------------------------------------------
vtkOGDFLayoutStrategy::vtkOGDFLayoutStrategy()
{
  this->LayoutModule = new ogdf::CircularLayout;
  this->LayoutEdges = false;
}

//-----------------------------------------------------------------------------
vtkOGDFLayoutStrategy::~vtkOGDFLayoutStrategy()
{
  this->SetLayoutModule(0);
}

//-----------------------------------------------------------------------------
void vtkOGDFLayoutStrategy::SetLayoutModule(ogdf::LayoutModule* layout)
{
  if (this->LayoutModule)
  {
    delete this->LayoutModule;
  }

  this->LayoutModule = layout;
}

//-----------------------------------------------------------------------------
ogdf::LayoutModule* vtkOGDFLayoutStrategy::GetLayoutModule() const
{
  return this->LayoutModule;
}

//-----------------------------------------------------------------------------
void vtkOGDFLayoutStrategy::SetLayoutModuleByName(const char* name)
{
  std::string str = name;
  std::transform(str.begin(), str.end(), str.begin(), tolower);
  str.erase(vtkstd::remove(str.begin(), str.end(), ' '), str.end());
  ogdf::LayoutModule* layout = 0;
  if (str == "balloon")
  {
    layout = new ogdf::BalloonLayout;
  }
  else if (str == "circular")
  {
    layout = new ogdf::CircularLayout;
  }
  else if (str == "davidsonharel")
  {
    layout = new ogdf::DavidsonHarelLayout;
  }
  else if (str == "dominance")
  {
    layout = new ogdf::DominanceLayout;
  }
  else if (str == "fmmm")
  {
    layout = new ogdf::FMMMLayout;
  }
  else if (str == "gem")
  {
    layout = new ogdf::GEMLayout;
  }
  else if (str == "mmmexamplefast")
  {
    layout = new ogdf::MMMExampleFastLayout;
  }
  else if (str == "mmmexamplenice")
  {
    layout = new ogdf::MMMExampleNiceLayout;
  }
  else if (str == "mmmexamplenotwist")
  {
    layout = new ogdf::MMMExampleNoTwistLayout;
  }
  else if (str == "fruchtermanreingold")
  {
    layout = new ogdf::SpringEmbedderFR;
  }
  else if (str == "kamadakawai")
  {
    layout = new ogdf::SpringEmbedderKK;
  }
  else if (str == "sugiyama")
  {
    layout = new ogdf::SugiyamaLayout;
  }

  if (layout)
  {
    this->SetLayoutModule(layout);
  }
}

//-----------------------------------------------------------------------------
void vtkOGDFLayoutStrategy::Layout()
{
  vtkDebugMacro(<< "vtkOGDFLayoutStrategy executing.");
  if (!this->LayoutModule)
  {
    vtkErrorMacro(<< "No layout module set.");
    return;
  }

  // Copy vtkGraph to ogdf::Graph
  std::map<vtkIdType, ogdf::node> nodeMap;
  ogdf::Graph G;
  ogdf::GraphAttributes GA(G);
  vtkIdType numVertices = this->Graph->GetNumberOfVertices();

  for (vtkIdType i = 0; i < numVertices; ++i)
  {
    nodeMap.insert(std::make_pair(i, G.newNode(i)));
  }

  vtkEdgeListIterator* edges = vtkEdgeListIterator::New();
  this->Graph->GetEdges(edges);
  while (edges->HasNext())
  {
    vtkEdgeType edge = edges->Next();
    G.newEdge(nodeMap[edge.Source], nodeMap[edge.Target], edge.Id);
  }
  edges->Delete();

  // Run layout
  this->LayoutModule->call(GA);

  // Copy ogdf::Graph's points to vtkPoints
  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(numVertices);
  for (ogdf::node v = G.firstNode(); v; v = v->succ())
  {
    points->SetPoint(v->index(), GA.x(v), GA.y(v), 0.0);
  }

  // Add the new points to the vtkGraph
  this->Graph->SetPoints(points);
  points->Delete();

  // Copy the edges points if required
  // CSM_TODO None of my ogdf layouts seem to return edge points so this is untested!
  if (this->LayoutEdges)
  {
    for (ogdf::edge e = G.firstEdge(); e; e = e->succ())
    {
      ogdf::DPolyline bends = GA.bends(e);
      vtkIdType numBends = bends.size();
      ogdf::DPolyline::const_iterator bendPoints(bends.begin());
      for (vtkIdType i = 0; i < numBends; ++i, ++bendPoints)
      {
        this->Graph->SetEdgePoint(e->index(), i, (*bendPoints).m_x,
            (*bendPoints).m_y, 0.0);
      }
    }
  }

  vtkDebugMacro(<< "OGDF layout complete.");
}

//-----------------------------------------------------------------------------
void vtkOGDFLayoutStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkGraphLayoutStrategy::PrintSelf(os, indent);
  os << indent << "LayoutModule: " << (this->LayoutModule ? "(set)" : "(none)")
          << endl;
  os << indent << "LayoutEdges: " << (this->LayoutEdges ? "On" : "Off") << endl;
}
