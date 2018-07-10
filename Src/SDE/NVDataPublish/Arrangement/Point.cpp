#include "Point.h"
#include "Line.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE::NVDataPublish::Arrangement;

//////////////////////////////////////////////////////////////////////////

void Node::addEdge(Edge* e, bool isBegin)
{
	isBegin ? m_begin.insert(e) : m_end.insert(e);
}

void Node::eraseEdge(Edge* e, bool isBegin)
{
	std::set<Edge*>::iterator itr;
	if (isBegin)
	{
		itr = m_begin.find(e);
		assert(itr != m_begin.end());
		m_begin.erase(itr);
	}
	else
	{
		itr = m_end.find(e);
		assert(itr != m_end.end());
		m_end.erase(itr);
	}
}

void Node::getAllEdges(std::vector<Edge*>* all) const
{
	all->reserve(m_begin.size() + m_end.size());

	std::set<Edge*>::const_iterator itr = m_begin.begin();
	for( ; itr != m_begin.end(); ++itr)
		all->push_back(*itr);

	itr = m_end.begin();
	for( ; itr != m_end.end(); ++itr)
		all->push_back(*itr);	
}

void Node::getUpAndLowEdges(std::vector<Edge*>* upIsP, std::vector<Edge*>* lowIsP) const
{
	std::set<Edge*>::const_iterator itr = m_begin.begin();
	for( ; itr != m_begin.end(); ++itr)
	{
		if ((*itr)->isBeginHigher())
			upIsP->push_back(*itr);
		else
			lowIsP->push_back(*itr);
	}

	itr = m_end.begin();
	for( ; itr != m_end.end(); ++itr)
	{
		if (!(*itr)->isBeginHigher())
			upIsP->push_back(*itr);
		else
			lowIsP->push_back(*itr);
	}
}

bool Node::merge(Node* n)
{
	assert(this != n);

	if (m_next == NULL && n->m_next == NULL)
	{
		m_next = n;
		n->m_next = this;
		m_prev = n;
		n->m_prev = this;
	}
	else if (m_next == NULL)
	{
		m_next = n->m_next;
		m_prev = n;
		n->m_next->m_prev = this;
		n->m_next = this;
	}
	else if (n->m_next == NULL)
	{
		n->m_next = m_next;
		n->m_prev = this;
		m_next->m_prev = n;
		m_next = n;
	}
	else
	{
		Node *thisPrev = m_prev, *nPrev = n->m_prev;
		m_prev = nPrev;
		nPrev->m_next = this;
		n->m_prev = thisPrev;
		thisPrev->m_next = n;
	}

	if (n->m_pos != m_pos)
	{
		n->setDirtyState();
		n->m_pos = m_pos;
		return true;
	}
	else
		return false;
}

void Node::setDirtyState()
{
	std::set<Edge*>::iterator itr = m_begin.begin();
	for( ; itr != m_begin.end(); ++itr)
		(*itr)->setDirtyState();

	itr = m_end.begin();
	for( ; itr != m_end.end(); ++itr)
		(*itr)->setDirtyState();
}

void Node::setType(const Rect& region, bool considerGeoAttr /* = true */)
{
	size_t inDegree = m_end.size(),
		outDegree = m_begin.size();

	if (inDegree + outDegree == 1)
	{
		m_type = END;
	}
	else if (inDegree + outDegree == 2)
	{
		std::vector<Edge*> edges;
		getAllEdges(&edges);

		if (edges.at(0)->m_pPolyEdge != edges.at(1)->m_pPolyEdge 
			&& !region.containsPoint(m_pos))
		{
			m_type = END;
			return;
		}

		if (considerGeoAttr)
		{
			if (edges.at(0)->isSameAttr(edges.at(1)))
				m_type = COMMON;
			else
				m_type = END;
		}
		else
		{
			m_type = COMMON;
		}

		if (m_type == COMMON && 
			edges.at(0)->m_pPolyEdge != edges.at(1)->m_pPolyEdge)
		{
			edges.at(0)->setDirtyState();
			edges.at(1)->setDirtyState();
		}
	}
	else if (inDegree + outDegree > 2)
	{
		m_type = INTERSECT;
		setDirtyState();
	}
	else
		assert(isSingle());
}

//////////////////////////////////////////////////////////////////////////

NodeMgr::~NodeMgr()
{
	while (!m_pool.empty())
	{
		Node* n = m_pool.top(); m_pool.pop();
		delete n;
	}
}

void NodeMgr::init(const Rect& computeRegion, const double& precision)
{
	m_grid.init(computeRegion, precision);
}

void NodeMgr::clear(bool usePool/* = false*/)
{
	m_grid.recycle(m_pool, usePool);
}

bool NodeMgr::createNode(const MapPos2D& p, Node** n)
{
	return m_grid.insertNode(p, n, &m_pool);
}

void NodeMgr::setAllNodesType(const Rect& region, bool considerGeoAttr /* = true */)
{
	MAP_GRID_TO_NODES::iterator itr = m_grid.m_mapGridToNodes.begin();
	for ( ; itr != m_grid.m_mapGridToNodes.end(); ++itr)
	{
		for (size_t i = 0; i < itr->second.size(); ++i)
			itr->second.at(i)->setType(region, considerGeoAttr);
	}
}

void NodeMgr::queryTopoNodesByRect(const MapScope& r, std::vector<Node*>* nodes) const
{
	m_grid.queryTopoNodesByRect(r, nodes);
}

//////////////////////////////////////////////////////////////////////////

void NodeMgr::RegularGrid::init(const Rect& computeRegion, const double& precision)
{
	m_layerInfo.init(computeRegion, precision);
	m_mapGridToNodes.clear();
}

bool NodeMgr::RegularGrid::insertNode(const MapPos2D& p, Node** n, std::stack<Node*>* pool)
{
	id_type id = getGridID(p);

	std::vector<id_type> nearbyGrids;
	getNearbyPos(id, &nearbyGrids);

	Node* pNearestNode		= NULL;
	double nearestDisSquare	= Tools::DOUBLE_MAX;
	MAP_GRID_TO_NODES::iterator itrNearestNode;

	for (size_t i = 0; i < nearbyGrids.size(); ++i)
	{
		MAP_GRID_TO_NODES::iterator itr = m_mapGridToNodes.find(nearbyGrids.at(i));
		if (itr != m_mapGridToNodes.end())
		{
			assert(itr->second.size() <= 1);
			for (size_t j = 0; j < itr->second.size(); ++j)
			{
				if (Tools::SpatialMath::isTheSamePos(p, itr->second.at(j)->pos(), m_layerInfo.gridEdgeLength))
				{
					double disSquare = Tools::SpatialMath::getDistanceWithSquare(p, itr->second.at(j)->pos());

					if (p == itr->second.at(j)->pos())
					{
						*n = itr->second.at(j);
						return false;
					}
					else if (disSquare < nearestDisSquare)
					{
						nearestDisSquare = disSquare;
						pNearestNode	 = itr->second.at(j);
						itrNearestNode	 = itr; 
					}
				}
			}
		}
	}

	if (pNearestNode != NULL)
	{
		*n = pNearestNode;
		return false;
	}
	else
	{
		if (!pool->empty())
		{
			*n = pool->top(); pool->pop();
		}
		else
		{
			*n = new Node;
		}

		(*n)->m_pos = p;
		std::vector<Node*> tmp;
		tmp.push_back(*n);
		m_mapGridToNodes.insert(std::make_pair(id, tmp));
		return true;
	}
}

void NodeMgr::RegularGrid::recycle(std::stack<Node*>& pool, bool usePool/* = false*/)
{
	MAP_GRID_TO_NODES::iterator itr = m_mapGridToNodes.begin();
	for ( ; itr != m_mapGridToNodes.end(); ++itr)
	{
		for (size_t i = 0; i < itr->second.size(); ++i)
		{
			Node* n = itr->second.at(i);
			if (usePool)
			{
				n->m_begin.clear();
				n->m_end.clear();
				pool.push(n);
			}
			else
				delete n;
		}
	}
}

void NodeMgr::RegularGrid::queryTopoNodesByRect(const MapScope& r, std::vector<Node*>* nodes) const
{
	MapPos2D start, end;
	start.d0 = r.m_xMin - m_layerInfo.gridEdgeLength < m_layerInfo.leftLowPos.d0 ?
		r.m_xMin : r.m_xMin - m_layerInfo.gridEdgeLength;
	start.d1 = r.m_yMin - m_layerInfo.gridEdgeLength < m_layerInfo.leftLowPos.d1 ?
		r.m_yMin : r.m_yMin - m_layerInfo.gridEdgeLength;
	end.d0 = r.m_xMax + m_layerInfo.gridEdgeLength;
	end.d1 = r.m_yMax + m_layerInfo.gridEdgeLength;

	MAP_GRID_TO_NODES::const_iterator 
		itrStart = m_mapGridToNodes.lower_bound(getGridID(start)),
		itrEnd = m_mapGridToNodes.upper_bound(getGridID(end));
	assert(itrStart != m_mapGridToNodes.end());
	for ( ; itrStart != itrEnd; ++itrStart)
	{
		for (size_t i = 0; i < itrStart->second.size(); ++i)
		{
			Node* n = itrStart->second.at(i);

			if (n->m_type == Node::COMMON)
				continue;

			if (Tools::SpatialMath::isPointInRect(n->pos(), r, true))
			{
				// left close, right open; down close, up open
				// If the point at the up or right bound of the all scope, 
				// right and up will also close.

				if (fabs(n->pos().d0 - m_layerInfo.rightTopPos.d0) > Tools::DOUBLE_TOLERANCE &&
					fabs(n->pos().d1 - m_layerInfo.rightTopPos.d1) > Tools::DOUBLE_TOLERANCE)
				{
					if (fabs(n->pos().d0 - r.m_xMax) < Tools::DOUBLE_TOLERANCE ||
						fabs(n->pos().d1 - r.m_yMax) < Tools::DOUBLE_TOLERANCE)
						continue;
				}

				nodes->push_back(n);
			}
		}
	}
}

id_type NodeMgr::RegularGrid::getGridID(const MapPos2D& p) const
{
	assert(p.d0 >= m_layerInfo.leftLowPos.d0 && p.d1 >= m_layerInfo.leftLowPos.d1);
	size_t	x = static_cast<size_t>(floor( (p.d0 - m_layerInfo.leftLowPos.d0) / m_layerInfo.gridEdgeLength )),
			y = static_cast<size_t>(floor( (p.d1 - m_layerInfo.leftLowPos.d1) / m_layerInfo.gridEdgeLength ));
	return x * m_layerInfo.yGridSize + y;
}

void NodeMgr::RegularGrid::getNearbyPos(const id_type& id, std::vector<id_type>* nearbyGrids) const
{
	id_type x = id / m_layerInfo.yGridSize, 
			y = id % m_layerInfo.yGridSize;

	nearbyGrids->push_back(id);
	if (x != 0)
	{
		if (y != 0)
			nearbyGrids->push_back( (x - 1) * m_layerInfo.yGridSize + y - 1 );
		nearbyGrids->push_back( (x - 1) * m_layerInfo.yGridSize + y );
		if (y != m_layerInfo.yGridSize - 1)
			nearbyGrids->push_back( (x - 1) * m_layerInfo.yGridSize + y + 1);
	}
	if (y != 0)
		nearbyGrids->push_back( x * m_layerInfo.yGridSize + y - 1 );
	if (y != m_layerInfo.yGridSize - 1)
		nearbyGrids->push_back( x * m_layerInfo.yGridSize + y + 1);
	if (x != m_layerInfo.xGridSize - 1)
	{
		if (y != 0)
			nearbyGrids->push_back( (x + 1) * m_layerInfo.yGridSize + y - 1 );
		nearbyGrids->push_back( (x + 1) * m_layerInfo.yGridSize + y );
		if (y != m_layerInfo.yGridSize - 1)
			nearbyGrids->push_back( (x + 1) * m_layerInfo.yGridSize + y + 1 );
	}
}

//////////////////////////////////////////////////////////////////////////

void NodeMgr::RegularGrid::LayerInfo::init(const Rect& computeRegion, const double& precision)
{
	gridEdgeLength	= precision;
	leftLowPos		= computeRegion.leftLow();
	rightTopPos		= computeRegion.rightTop();
	xGridSize		= static_cast<size_t>( ceil(computeRegion.xLength() / gridEdgeLength) );
	yGridSize		= static_cast<size_t>( ceil(computeRegion.yLength() / gridEdgeLength) );
}
