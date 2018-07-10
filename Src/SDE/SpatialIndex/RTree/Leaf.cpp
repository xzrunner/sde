#include "Node.h"
#include "RTree.h"
#include "Leaf.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::RTree;

Leaf::~Leaf()
{
}

Leaf::Leaf(IS_SDE::SpatialIndex::RTree::RTree* pTree, id_type id)
	: Node(pTree, id, 0, pTree->m_leafCapacity)
{
}

NodePtr Leaf::chooseSubtree(const Rect& mbr, size_t level, std::stack<id_type>& pathBuffer)
{
	// should make sure to relinquish other PoolPointer lists that might be pointing to the
	// same leaf.
	return NodePtr(this, &(m_pRTree->m_leafPool));
}

NodePtr Leaf::findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer)
{
	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		// should make sure to relinquish other PoolPointer lists that might be pointing to the
		// same leaf.
		if (m_pIdentifier[cChild] == id && mbr == *(m_ptrMBR[cChild])) return NodePtr(this, &(m_pRTree->m_leafPool));
	}

	return NodePtr();
}

void Leaf::split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& pLeft, NodePtr& pRight)
{
	m_pRTree->m_stats.m_splits++;

	std::vector<size_t> g1, g2;

	switch (m_pRTree->m_treeVariant)
	{
	case RV_LINEAR:
	case RV_QUADRATIC:
		rtreeSplit(dataLength, pData, mbr, id, g1, g2);
		break;
	case RV_RSTAR:
		rstarSplit(dataLength, pData, mbr, id, g1, g2);
		break;
	default:
		throw Tools::NotSupportedException("Leaf::split: Tree variant not supported.");
	}

	pLeft = m_pRTree->m_leafPool.acquire();
	pRight = m_pRTree->m_leafPool.acquire();

	if (pLeft.get() == 0) pLeft = NodePtr(new Leaf(m_pRTree, -1), &(m_pRTree->m_leafPool));
	if (pRight.get() == 0) pRight = NodePtr(new Leaf(m_pRTree, -1), &(m_pRTree->m_leafPool));

	pLeft->m_nodeMBR = m_pRTree->m_infiniteRect;
	pRight->m_nodeMBR = m_pRTree->m_infiniteRect;

	size_t cIndex;

	for (cIndex = 0; cIndex < g1.size(); cIndex++)
	{
		pLeft->insertEntry(m_pDataLength[g1[cIndex]], m_pData[g1[cIndex]], *(m_ptrMBR[g1[cIndex]]), m_pIdentifier[g1[cIndex]]);
		// we don't want to delete the data array from this node's destructor!
		m_pData[g1[cIndex]] = 0;
	}

	for (cIndex = 0; cIndex < g2.size(); cIndex++)
	{
		pRight->insertEntry(m_pDataLength[g2[cIndex]], m_pData[g2[cIndex]], *(m_ptrMBR[g2[cIndex]]), m_pIdentifier[g2[cIndex]]);
		// we don't want to delete the data array from this node's destructor!
		m_pData[g2[cIndex]] = 0;
	}
}

void Leaf::deleteData(id_type id, std::stack<id_type>& pathBuffer)
{
	size_t child;

	for (child = 0; child < m_children; child++)
	{
		if (m_pIdentifier[child] == id) break;
	}

	deleteEntry(child);
	m_pRTree->writeNode(this);

	std::stack<NodePtr> toReinsert;
	NodePtr ptrThis(this, &(m_pRTree->m_leafPool));
	condenseTree(toReinsert, pathBuffer, ptrThis);
	ptrThis.relinquish();

	// re-insert eliminated nodes.
	while (! toReinsert.empty())
	{
		NodePtr n = toReinsert.top(); toReinsert.pop();
		m_pRTree->deleteNode(n.get());

		for (size_t cChild = 0; cChild < n->m_children; cChild++)
		{
			// keep this in the for loop. The tree height might change after insertions.
			byte* overflowTable = new byte[m_pRTree->m_stats.m_treeHeight];
			bzero(overflowTable, m_pRTree->m_stats.m_treeHeight);
			m_pRTree->insertData_impl(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], n->m_level, overflowTable);
			n->m_pData[cChild] = 0;
			delete[] overflowTable;
		}
		if (n.get() == this) n.relinquish();
	}
}
