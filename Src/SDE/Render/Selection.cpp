#include "Selection.h"
#include "MapLayer.h"
#include "../SpatialIndex/Utility/Visitor.h"

using namespace IS_SDE;
using namespace IS_SDE::Render;

void Selection::insert(MapLayer* mapLayer, id_type nodeID, id_type objID)
{
	MAP_LAYER_TO_OBJS::iterator itrLayer = m_selection.find(mapLayer);
	if (itrLayer == m_selection.end())
	{
		SET_ALL_OBJS_IN_LAYER newLayerObjs;
		newLayerObjs.insert(SelectionObj(nodeID, objID));
		m_selection.insert(std::make_pair(mapLayer, newLayerObjs));
	}
	else
	{
		SelectionObj obj(nodeID, objID);
		std::pair<SET_ALL_OBJS_IN_LAYER::iterator, bool> insertState
			= itrLayer->second.insert(obj);
	}
}

void Selection::insert(MapLayer* mapLayer, const SpatialIndex::QuerySelectionVisitor& vis)
{
	MAP_LAYER_TO_OBJS::iterator itrLayer = m_selection.find(mapLayer);
	if (itrLayer == m_selection.end())
	{
		SET_ALL_OBJS_IN_LAYER newLayerObjs;

		size_t nodeCount = vis.m_nodeIDWithObjID.size();
		for (size_t i = 0; i < nodeCount; ++i)
		{
			id_type nodeID = vis.m_nodeIDWithObjID.at(i).first;
			size_t objCount = vis.m_nodeIDWithObjID.at(i).second.size();
			for (size_t j = 0; j < objCount; ++j)
			{
				newLayerObjs.insert(SelectionObj(nodeID, vis.m_nodeIDWithObjID.at(i).second.at(j)));
			}
		}
		m_selection.insert(std::make_pair(mapLayer, newLayerObjs));
	}
	else
	{
		size_t nodeCount = vis.m_nodeIDWithObjID.size();
		for (size_t i = 0; i < nodeCount; ++i)
		{
			id_type nodeID = vis.m_nodeIDWithObjID.at(i).first;
			size_t objCount = vis.m_nodeIDWithObjID.at(i).second.size();
			for (size_t j = 0; j < objCount; ++j)
			{
				SelectionObj obj(nodeID, vis.m_nodeIDWithObjID.at(i).second.at(j));
				std::pair<SET_ALL_OBJS_IN_LAYER::iterator, bool> insertState
					= itrLayer->second.insert(obj);
			}
		}
	}
}

void Selection::deleteLayer(MapLayer* mapLayer)
{
	MAP_LAYER_TO_OBJS::iterator itrLayer = m_selection.find(mapLayer);
	if (itrLayer != m_selection.end())
	{
		m_selection.erase(itrLayer);
	}
}

void Selection::getAllShapes(std::vector<IShape*>& shapes) const
{
	MAP_LAYER_TO_OBJS::const_iterator itrLayer = m_selection.begin();
	for ( ; itrLayer != m_selection.end(); ++itrLayer)
	{
		ISpatialIndex* index = itrLayer->first->getSpatialIndex();

		SET_ALL_OBJS_IN_LAYER::const_iterator itrObj = itrLayer->second.begin();
		for ( ; itrObj != itrLayer->second.end(); ++itrObj)
		{
			IShape* s; 
			index->readData(itrObj->m_nodeID, itrObj->m_objID, &s);
			if (s)
				shapes.push_back(s);
		}
	}
}

void Selection::getAllShapes(std::vector<IShape*>& shapes, std::vector<ISpatialIndex*>& indexes, 
							 std::vector<id_type>& objIDs) const
{
	MAP_LAYER_TO_OBJS::const_iterator itrLayer = m_selection.begin();
	for ( ; itrLayer != m_selection.end(); ++itrLayer)
	{
		ISpatialIndex* index = itrLayer->first->getSpatialIndex();

		SET_ALL_OBJS_IN_LAYER::const_iterator itrObj = itrLayer->second.begin();
		for ( ; itrObj != itrLayer->second.end(); ++itrObj)
		{
			IShape* s; 
			index->readData(itrObj->m_nodeID, itrObj->m_objID, &s);
			if (s)
			{
				shapes.push_back(s);
				indexes.push_back(index);
				objIDs.push_back(itrObj->m_objID);
			}
		}
	}
}

Selection::SelectionObj::SelectionObj(id_type nodeID, id_type objID)
	: m_nodeID(nodeID), m_objID(objID)
{
}

bool Selection::SelectionObjComp::operator() (
	const Selection::SelectionObj& lhs, 
	const Selection::SelectionObj& rhs
	) const
{
	return lhs.m_objID < rhs.m_objID;
}
