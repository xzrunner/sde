#include "Toolbox.h"
#include "../../Algorithms/ComputationalGeometry/DelaunayTriangulation.h"
#include "../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Agent;

Toolbox::~Toolbox()
{
	std::map<ToolType, void*>::iterator itr = m_tools.begin();
	for ( ; itr != m_tools.end(); ++itr)
		deleteTool(itr->first, itr->second);
}

void Toolbox::addTool(ToolType type, void* tool)
{
	std::map<ToolType, void*>::iterator itr = m_tools.find(type);
	if (itr != m_tools.end())
	{
		deleteTool(type, itr->second);
		itr->second = tool;
	}
	else
		m_tools.insert(std::make_pair(type, tool));
}

void* Toolbox::fetchTool(ToolType type) const
{
	std::map<ToolType, void*>::const_iterator itr = m_tools.find(type);
	if (itr != m_tools.end())
		return itr->second;
	else
		return NULL;
}

void Toolbox::deleteTool(ToolType type, void* tool)
{
	switch (type)
	{
	case TT_DELAUNAY_TRIANGULATION:
		delete static_cast<Algorithms::DelaunayTriangulation*>(tool);
		break;
	case TT_VORONOI:
		delete static_cast<Algorithms::VoronoiByDelaunay*>(tool);
		break;
	case TT_SPATIAL_INDEX:
		delete static_cast<ISpatialIndex*>(tool);
		break;
	case TT_SPATIAL_INDEXES:
		delete static_cast<std::vector<ISpatialIndex*>*>(tool);
		break;
	default:
		assert(0);
	}
}