#include "Index.h"
#include "MapPartition.h"
#include "Leaf.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

Index::~Index()
{
	for (size_t i = 0; i < 4; ++i)
		delete m_child[i];
}

void Index::split(PartitionMap& pm)
{
	Rect childScope(m_scope);
	for (size_t i = 0; i < 4; ++i)
	{
		Tools::SpatialMath::getRectChildGrid(m_scope, i, childScope);

		if (shouldSplitNode(pm, childScope))
		{
			m_child[i] = new Index(childScope);
			m_child[i]->split(pm);
		}
		else
		{
			m_child[i] = new Leaf(childScope);
			pm.m_allLeaves.push_back(m_child[i]);
		}
	}
}

void Index::queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const
{
	assert(PartitionMap::isPointInScope(p, m_scope));

	size_t pos = Tools::SpatialMath::getPositionPosInRect(m_scope, p);
	m_child[pos]->queryAdjacencyListID(p, IDs);
}

void Index::setNodeCount()
{
	if (m_nodeCount != 0)
		return;
	else
	{
		for (size_t i = 0; i < 4; ++i)
		{
			m_child[i]->setNodeCount();
			m_nodeCount += m_child[i]->m_nodeCount;
		}
	}
}

bool Index::shouldSplitNode(const PartitionMap& pm, const Rect& scope) const
{
	return pm.shouldSplitNode(scope);
}