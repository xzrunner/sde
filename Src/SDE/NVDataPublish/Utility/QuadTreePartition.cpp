#include "QuadTreePartition.h"
#include "../Network/AdjacencyList.h"
#include "../../SimulateNavigate/Network/SLSpatialIndex.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Utility;

QPNode::QPNode(SrcNode* src)
	: m_srcNode(src), m_scope(src->m_scope)
{
	if (m_srcNode->m_bLeaf)
	{
		SrcLeaf* leaf = dynamic_cast<SrcLeaf*>(m_srcNode);
		for (size_t i = 0; i < leaf->m_pageCount; ++i)
			m_singleALPageIDs.push_back(leaf->m_startALPage + i);
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = NULL;
	}
	else
	{
		SrcIndex* index = dynamic_cast<SrcIndex*>(m_srcNode);
		assert(index != NULL);
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = new QPNode(index->m_child[i]);
	}
}

QPNode::QPNode(const Rect& r, const std::vector<size_t>& dataPageIDs)
	: m_srcNode(NULL), m_scope(r), m_singleALPageIDs(dataPageIDs)
{
	for (size_t i = 0; i < 4; ++i)
		m_child[i] = NULL;
}

QPNode::~QPNode()
{
	for (size_t i = 0; i < 4; ++i)
		delete m_child[i];
}

void QPNode::reconstruct(IStorageManager* sm, size_t capacity)
{
	if (m_srcNode->m_topoNodeCount < capacity)
	{
		if (!isLeaf())
			condense();
	}
	else
	{
		if (!isLeaf())
			for (size_t i = 0; i < 4; ++i)
				m_child[i]->reconstruct(sm, capacity);
		else
		{
			assert(m_allTopoNodePos.empty());
			m_allTopoNodePos.clear();
			SrcLeaf* l = dynamic_cast<SrcLeaf*>(m_srcNode);
			assert(l != NULL);
			for (size_t i = 0; i < l->m_pageCount; ++i)
			{
				byte* data;
				size_t len;
				sm->loadByteArray(l->m_startALPage + i, len, &data);
				Network::AdjacencyList::fetchAllNodesPos(data, m_allTopoNodePos);
				delete[] data;
			}
			assert(m_allTopoNodePos.size() == m_srcNode->m_topoNodeCount);
			split(capacity, m_singleALPageIDs);
		}
	}
}

void QPNode::condense()
{
	for (size_t i = 0; i < 4; ++i)
	{
		QPNode* child = m_child[i];
		if (!child->isLeaf())
			child->condense();
		std::copy(child->m_singleALPageIDs.begin(), child->m_singleALPageIDs.end(), back_inserter(m_singleALPageIDs));
		delete child, m_child[i] = NULL;
	}
}

void QPNode::split(size_t capacity, const std::vector<size_t>& dataPageIDs)
{
	if (m_allTopoNodePos.size() <= capacity)
		return;

	Rect childScope(m_scope);
	for (size_t i = 0; i < 4; ++i)
	{
		Tools::SpatialMath::getRectChildGrid(m_scope, i, childScope);
		m_child[i] = new QPNode(childScope, dataPageIDs);
	}

	for (size_t i = 0; i < m_allTopoNodePos.size(); ++i)
	{
		MapPos2D& p(m_allTopoNodePos[i]);
		size_t pos = Tools::SpatialMath::getPositionPosInRect(m_scope, p);
		m_child[pos]->m_allTopoNodePos.push_back(p);
	}

	for (size_t i = 0; i < 4; ++i)
		m_child[i]->split(capacity, dataPageIDs);
}
