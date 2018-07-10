#include "RTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::RTree;

Index::~Index()
{
}

Index::Index(SpatialIndex::RTree::RTree* pTree, id_type id, size_t level) 
	: Node(pTree, id, level, pTree->m_indexCapacity)
{
}

NodePtr Index::chooseSubtree(const Rect& mbr, size_t insertionLevel, std::stack<id_type>& pathBuffer)
{
	if (m_level == insertionLevel) return NodePtr(this, &(m_pRTree->m_indexPool));

	pathBuffer.push(m_identifier);

	size_t child = 0;

	switch (m_pRTree->m_treeVariant)
	{
	case RV_LINEAR:
	case RV_QUADRATIC:
		child = findLeastEnlargement(mbr);
		break;
	case RV_RSTAR:
		if (m_level == 1)
		{
			// if this node points to leaves...
			child = findLeastOverlap(mbr);
		}
		else
		{
			child = findLeastEnlargement(mbr);
		}
		break;
	default:
		throw Tools::NotSupportedException("Index::chooseSubtree: Tree variant not supported.");
	}
	assert(child != Tools::UINT32_MAX);

	NodePtr n = m_pRTree->readNode(m_pIdentifier[child]);
	NodePtr ret = n->chooseSubtree(mbr, insertionLevel, pathBuffer);
	assert(n.unique());
	if (ret.get() == n.get()) n.relinquish();

	return ret;
}

NodePtr Index::findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer)
{
	pathBuffer.push(m_identifier);

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		if (m_ptrMBR[cChild]->containsRect(mbr))
		{
			NodePtr n = m_pRTree->readNode(m_pIdentifier[cChild]);
			NodePtr l = n->findLeaf(mbr, id, pathBuffer);
			if (n.get() == l.get()) n.relinquish();
			if (l.get() != 0) return l;
		}
	}

	pathBuffer.pop();

	return NodePtr();
}

void Index::split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& ptrLeft, NodePtr& ptrRight)
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
		throw Tools::NotSupportedException("Index::split: Tree variant not supported.");
	}

	ptrLeft = m_pRTree->m_indexPool.acquire();
	ptrRight = m_pRTree->m_indexPool.acquire();

	if (ptrLeft.get() == 0) ptrLeft = NodePtr(new Index(m_pRTree, m_identifier, m_level), &(m_pRTree->m_indexPool));
	if (ptrRight.get() == 0) ptrRight = NodePtr(new Index(m_pRTree, -1, m_level), &(m_pRTree->m_indexPool));

	ptrLeft->m_nodeMBR = m_pRTree->m_infiniteRect;
	ptrRight->m_nodeMBR = m_pRTree->m_infiniteRect;

	size_t cIndex;

	for (cIndex = 0; cIndex < g1.size(); cIndex++)
	{
		ptrLeft->insertEntry(0, 0, *(m_ptrMBR[g1[cIndex]]), m_pIdentifier[g1[cIndex]]);
	}

	for (cIndex = 0; cIndex < g2.size(); cIndex++)
	{
		ptrRight->insertEntry(0, 0, *(m_ptrMBR[g2[cIndex]]), m_pIdentifier[g2[cIndex]]);
	}
}

size_t Index::findLeastEnlargement(const Rect& r) const
{
	double area = Tools::DOUBLE_MAX;
	size_t best = Tools::UINT32_MAX;

	RectPtr t = m_pRTree->m_rectPool.acquire();

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		m_ptrMBR[cChild]->getCombinedRect(*t, r);

		double a = m_ptrMBR[cChild]->getArea();
		double enl = t->getArea() - a;

		if (enl < area)
		{
			area = enl;
			best = cChild;
		}
		else if (enl == area)
		{
			// this will rarely happen, so compute best area on the fly only
			// when necessary.
			if (a < m_ptrMBR[best]->getArea()) best = cChild;
		}
	}

	return best;
}

size_t Index::findLeastOverlap(const Rect& r) const
{
	OverlapEntry** entries = new OverlapEntry*[m_children];

	double leastOverlap = Tools::DOUBLE_MAX;
	double me = Tools::DOUBLE_MAX;
	OverlapEntry* best = 0;

	// find combined region and enlargement of every entry and store it.
	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		try
		{
			entries[cChild] = new OverlapEntry();
		}
		catch (...)
		{
			for (size_t i = 0; i < cChild; i++) delete entries[i];
			delete[] entries;
			throw;
		}

		entries[cChild]->m_index = cChild;
		entries[cChild]->m_original = m_ptrMBR[cChild];
		entries[cChild]->m_combined = m_pRTree->m_rectPool.acquire();
		m_ptrMBR[cChild]->getCombinedRect(*(entries[cChild]->m_combined), r);
		entries[cChild]->m_oa = entries[cChild]->m_original->getArea();
		entries[cChild]->m_ca = entries[cChild]->m_combined->getArea();
		entries[cChild]->m_enlargement = entries[cChild]->m_ca - entries[cChild]->m_oa;

		if (entries[cChild]->m_enlargement < me)
		{
			me = entries[cChild]->m_enlargement;
			best = entries[cChild];
		}
		else if (entries[cChild]->m_enlargement == me && entries[cChild]->m_oa < best->m_oa)
		{
			best = entries[cChild];
		}
	}

	if (me < -Tools::DOUBLE_TOLERANCE || me > Tools::DOUBLE_TOLERANCE)
	{
		size_t cIterations;

		if (m_children > m_pRTree->m_nearMinimumOverlapFactor)
		{
			// sort entries in increasing order of enlargement.
			::qsort(entries, m_children,
				sizeof(OverlapEntry*),
				OverlapEntry::compareEntries);
			assert(entries[0]->m_enlargement <= entries[m_children - 1]->m_enlargement);

			cIterations = m_pRTree->m_nearMinimumOverlapFactor;
		}
		else
		{
			cIterations = m_children;
		}

		// calculate overlap of most important original entries (near minimum overlap cost).
		for (size_t cIndex = 0; cIndex < cIterations; cIndex++)
		{
			double dif = 0.0;
			OverlapEntry* e = entries[cIndex];

			for (size_t cChild = 0; cChild < m_children; cChild++)
			{
				if (e->m_index != cChild)
				{
					double f = e->m_combined->getIntersectingArea(*(m_ptrMBR[cChild]));
					if (f != 0.0) dif += f - e->m_original->getIntersectingArea(*(m_ptrMBR[cChild]));
				}
			} // for (cChild)

			if (dif < leastOverlap)
			{
				leastOverlap = dif;
				best = entries[cIndex];
			}
			else if (dif == leastOverlap)
			{
				if (e->m_enlargement == best->m_enlargement)
				{
					// keep the one with least area.
					if (e->m_original->getArea() < best->m_original->getArea()) best = entries[cIndex];
				}
				else
				{
					// keep the one with least enlargement.
					if (e->m_enlargement < best->m_enlargement) best = entries[cIndex];
				}
			}
		} // for (cIndex)
	}

	size_t ret = best->m_index;

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		delete entries[cChild];
	}
	delete[] entries;

	return ret;
}

void Index::adjustTree(Node* n, std::stack<id_type>& pathBuffer)
{
	m_pRTree->m_stats.m_adjustments++;

	// find entry pointing to old node;
	size_t child;
	for (child = 0; child < m_children; child++)
	{
		if (m_pIdentifier[child] == n->m_identifier) break;
	}

	// MBR needs recalculation if either:
	//   1. the NEW child MBR is not contained.
	//   2. the OLD child MBR is touching.
	bool bContained = m_nodeMBR.containsRect(n->m_nodeMBR);
	bool bTouches = m_nodeMBR.touchesRect(*(m_ptrMBR[child]));
	bool bRecompute = (! bContained || (bTouches && m_pRTree->m_bTightMBRs));

	*(m_ptrMBR[child]) = n->m_nodeMBR;

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

	m_pRTree->writeNode(this);

	if (bRecompute && (! pathBuffer.empty()))
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pRTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);
	}
}

void Index::adjustTree(Node* n1, Node* n2, std::stack<id_type>& pathBuffer, byte* overflowTable)
{
	m_pRTree->m_stats.m_adjustments++;

	// find entry pointing to old node;
	size_t child;
	for (child = 0; child < m_children; child++)
	{
		if (m_pIdentifier[child] == n1->m_identifier) break;
	}

	// MBR needs recalculation if either:
	//   1. the NEW child MBR is not contained.
	//   2. the OLD child MBR is touching.
	bool bContained = m_nodeMBR.containsRect(n1->m_nodeMBR);
	bool bTouches = m_nodeMBR.touchesRect(*(m_ptrMBR[child]));
	bool bRecompute = (! bContained || (bTouches && m_pRTree->m_bTightMBRs));

	*(m_ptrMBR[child]) = n1->m_nodeMBR;

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

	// No write necessary here. insertData will write the node if needed.
	//m_pRTree->writeNode(this);

	bool bAdjusted = insertData(0, 0, n2->m_nodeMBR, n2->m_identifier, pathBuffer, overflowTable);

	// if n2 is contained in the node and there was no split or reinsert,
	// we need to adjust only if recalculation took place.
	// In all other cases insertData above took care of adjustment.
	if ((! bAdjusted) && bRecompute && (! pathBuffer.empty()))
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pRTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);
	}
}
