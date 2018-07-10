#include "MapPartition.h"
#include "../../SpatialIndex/NVRTree/NVRTree.h"
#include "../../SpatialIndex/Utility/QueryStrategy.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Arrangement;

PartitionMap::PartitionMap(const std::vector<ISpatialIndex*>& src, size_t maxNum)
	: m_root(NULL), m_maxNum(maxNum)
{
	m_layers.reserve(src.size());
	for (size_t i = 0; i < src.size(); ++i)
	{
		SpatialIndex::NVRTree::NVRTree* tree 
			= dynamic_cast<SpatialIndex::NVRTree::NVRTree*>(src.at(i));
		if (!tree)
		{
			assert(tree);
			throw Tools::IllegalArgumentException(
				"PartitionMap::PartitionMap: should use NVRTree spatial index."
				);
		}
		m_layers.push_back(tree);
	}

	build();
}

PartitionMap::~PartitionMap()
{
	delete m_root;
}

void PartitionMap::build()
{
	Rect totScope;
	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		SpatialIndex::LayerRegionQueryStrategy qs;
		m_layers.at(i)->queryStrategy(qs);
		totScope.combineRect(qs.m_indexedSpace);
	}

	if (shouldSplitGrid(totScope))
	{
		m_root = new Grid(totScope, false);
		m_root->split(*this);
	}
	else
	{
		m_root = new Grid(totScope, true);
		m_allLeaves.push_back(m_root);
	}
}

bool PartitionMap::shouldSplitGrid(const Rect& scope) const
{
	CountRoadVisitor vis(m_maxNum);

	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		if (m_layers.at(i)->countIntersectsQuery(scope, vis))
			return true;
	}
	return false;
}

//
//	class PartitionMap::Grid
//

PartitionMap::Grid::Grid(const Rect& scope, bool bLeaf)
	: m_scope(scope), m_bLeaf(bLeaf)
{
}

PartitionMap::Grid::~Grid()
{
	if (!m_bLeaf)
	{
		for (size_t i = 0; i < 4; ++i)
			delete m_children[i];
	}
}

void PartitionMap::Grid::split(PartitionMap& pm)
{
	Rect childScope(m_scope);
	for (size_t i = 0; i < 4; ++i)
	{
		Tools::SpatialMath::getRectChildGrid(m_scope, i, childScope);

		if (pm.shouldSplitGrid(childScope))
		{
			m_children[i] = new Grid(childScope, false);
			m_children[i]->split(pm);
		}
		else
		{
			m_children[i] = new Grid(childScope, true);
			pm.m_allLeaves.push_back(m_children[i]);
		}
	}
}

//
// class PartitionMap::CountRoadVisitor
//

void PartitionMap::CountRoadVisitor::countNode(const INode& node)
{
	m_count += node.getChildrenCount();	
}

void PartitionMap::CountRoadVisitor::countData(const INode& node, size_t iData, const IShape& query)
{
	++m_count;
}

bool PartitionMap::CountRoadVisitor::overUpperLimit() const
{
	return m_count > m_upperLimit;
}