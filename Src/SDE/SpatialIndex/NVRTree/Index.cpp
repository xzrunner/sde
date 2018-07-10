#include "NVRTree.h"
#include "Node.h"
#include "Index.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::NVRTree;

Index::~Index()
{
}

Index::Index(SpatialIndex::NVRTree::NVRTree* pTree, id_type id, size_t level)
	: Node(pTree, id, level, pTree->m_indexCapacity)
{
}

bool Index::findLeafPath(id_type nodeID, const Rect& nodeMBR, 
						 std::stack<std::pair<id_type, size_t> >* pathBuffer)
{
	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		pathBuffer->push(std::make_pair(m_identifier, cChild));
		if (m_ptrMBR[cChild]->containsRect(nodeMBR))
		{
			NodePtr n = m_pNVRTree->readNode(m_pIdentifier[cChild]);
			bool bFind = n->findLeafPath(nodeID, nodeMBR, pathBuffer);
			if (bFind)
				return bFind;
		}
		pathBuffer->pop();
	}

	return false;
}

void Index::adjustTree(Node* n, size_t index, 
					   std::stack<std::pair<id_type, size_t> >* pathBuffer)
{
	// MBR needs recalculation if either:
	//   1. the NEW child MBR is not contained.
	//   2. the OLD child MBR is touching.
	bool bContained = m_nodeMBR.containsRect(n->m_nodeMBR);
	bool bTouches = m_nodeMBR.touchesRect(*(m_ptrMBR[index]));
	bool bRecompute = (!bContained || bTouches);

	*(m_ptrMBR[index]) = n->m_nodeMBR;

	if (bRecompute)
	{
		for (size_t cDim = 0; cDim < m_nodeMBR.DIMENSION; cDim++)
		{
			m_nodeMBR.m_pLow[cDim] = Tools::DOUBLE_MAX;
			m_nodeMBR.m_pHigh[cDim] = -Tools::DOUBLE_MAX;

			for (size_t cChild = 0; cChild < m_children; cChild++)
			{
				m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim]);
				m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim]);
			}
		}
	}

	m_pNVRTree->writeNode(this);

	if (bRecompute && !pathBuffer->empty())
	{
		std::pair<id_type, size_t> pos = pathBuffer->top(); pathBuffer->pop();
		NodePtr ptrN = m_pNVRTree->readNode(pos.first);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pos.second, pathBuffer);
	}
}