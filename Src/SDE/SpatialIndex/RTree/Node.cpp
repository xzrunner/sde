#include "Node.h"
#include "RTree.h"
#include "Index.h"
#include "../../BasicType/Point.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::RTree;

//
// SpatialIndex::INode interface
//

bool Node::isLeaf() const
{
	return (m_level == 0);
}

bool Node::isIndex() const
{
	return (m_level != 0);
}

size_t Node::getNodeType() const
{
	if (m_level == 0)
		return PersistentLeaf;
	else
		return PersistentIndex;
}

//
// Internal
//

Node::Node() 
	: BaseTreeNode(), m_pRTree(NULL)
{
}

Node::Node(IS_SDE::SpatialIndex::RTree::RTree* pTree, IS_SDE::id_type id, size_t level, size_t capacity) 
	: BaseTreeNode(id, level, capacity), m_pRTree(pTree)
{
}

Node& Node::operator = (const Node& n)
{
	throw Tools::IllegalStateException("operator =: This should never be called.");
}

void Node::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	BaseTreeNode::insertEntry(dataLength, pData, mbr, id);
	m_nodeMBR.combineRect(mbr);
}

void Node::deleteEntry(size_t index)
{
	assert(index >= 0 && index < m_children);

	// cache it, since I might need it for "touches" later.
	RectPtr ptrR = m_ptrMBR[index];

	m_totalDataLength -= m_pDataLength[index];
	if (m_pData[index] != 0) delete[] m_pData[index];

	if (m_children > 1 && index != m_children - 1)
	{
		m_pDataLength[index] = m_pDataLength[m_children - 1];
		m_pData[index] = m_pData[m_children - 1];
		m_ptrMBR[index] = m_ptrMBR[m_children - 1];
		m_pIdentifier[index] = m_pIdentifier[m_children - 1];
	}

	m_children--;

	// WARNING: index has now changed. Do not use it below here.

	if (m_children == 0)
	{
		m_nodeMBR = m_pRTree->m_infiniteRect;
	}
	else if (m_pRTree->m_bTightMBRs && m_nodeMBR.touchesRect(*ptrR))
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
}

void Node::clearAllEntry(bool invalidateRegion/* = true*/)
{
	for (size_t i = 0; i < m_children; ++i)
	{
		delete[] m_pData[i];
	}
	m_totalDataLength = 0;
	m_children = 0;
	if (invalidateRegion)
		m_nodeMBR = m_pRTree->m_infiniteRect;
}

bool Node::insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer, byte* overflowTable)
{
	if (m_children < m_capacity)
	{
		bool adjusted = false;

		// this has to happen before insertEntry modifies m_nodeMBR.
		bool b = m_nodeMBR.containsRect(mbr);

		insertEntry(dataLength, pData, mbr, id);
		m_pRTree->writeNode(this);

		if ((! b) && (! pathBuffer.empty()))
		{
			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pRTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(this, pathBuffer);
			adjusted = true;
		}

		return adjusted;
	}
	else if (m_pRTree->m_treeVariant == RV_RSTAR && (! pathBuffer.empty()) && overflowTable[m_level] == 0)
	{
		overflowTable[m_level] = 1;

		std::vector<size_t> vReinsert, vKeep;
		reinsertData(dataLength, pData, mbr, id, vReinsert, vKeep);

		size_t lReinsert = vReinsert.size();
		size_t lKeep = vKeep.size();

		byte** reinsertdata = 0;
		RectPtr* reinsertmbr = 0;
		id_type* reinsertid = 0;
		size_t* reinsertlen = 0;
		byte** keepdata = 0;
		RectPtr* keepmbr = 0;
		id_type* keepid = 0;
		size_t* keeplen = 0;

		try
		{
			reinsertdata = new byte*[lReinsert];
			reinsertmbr = new RectPtr[lReinsert];
			reinsertid = new id_type[lReinsert];
			reinsertlen = new size_t[lReinsert];

			keepdata = new byte*[m_capacity + 1];
			keepmbr = new RectPtr[m_capacity + 1];
			keepid = new id_type[m_capacity + 1];
			keeplen = new size_t[m_capacity + 1];
		}
		catch (...)
		{
			delete[] reinsertdata;
			delete[] reinsertmbr;
			delete[] reinsertid;
			delete[] reinsertlen;
			delete[] keepdata;
			delete[] keepmbr;
			delete[] keepid;
			delete[] keeplen;
			throw;
		}

		size_t cIndex;

		for (cIndex = 0; cIndex < lReinsert; cIndex++)
		{
			reinsertlen[cIndex] = m_pDataLength[vReinsert[cIndex]];
			reinsertdata[cIndex] = m_pData[vReinsert[cIndex]];
			reinsertmbr[cIndex] = m_ptrMBR[vReinsert[cIndex]];
			reinsertid[cIndex] = m_pIdentifier[vReinsert[cIndex]];
		}

		for (cIndex = 0; cIndex < lKeep; cIndex++)
		{
			keeplen[cIndex] = m_pDataLength[vKeep[cIndex]];
			keepdata[cIndex] = m_pData[vKeep[cIndex]];
			keepmbr[cIndex] = m_ptrMBR[vKeep[cIndex]];
			keepid[cIndex] = m_pIdentifier[vKeep[cIndex]];
		}

		delete[] m_pDataLength;
		delete[] m_pData;
		delete[] m_ptrMBR;
		delete[] m_pIdentifier;

		m_pDataLength = keeplen;
		m_pData = keepdata;
		m_ptrMBR = keepmbr;
		m_pIdentifier = keepid;
		m_children = lKeep;
		m_totalDataLength = 0;

		for (size_t cChild = 0; cChild < m_children; cChild++) m_totalDataLength += m_pDataLength[cChild];

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

		m_pRTree->writeNode(this);

		// Divertion from R*-Tree algorithm here. First adjust
		// the path to the root, then start reinserts, to avoid complicated handling
		// of changes to the same node from multiple insertions.
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = m_pRTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(this, pathBuffer);

		for (cIndex = 0; cIndex < lReinsert; cIndex++)
		{
			m_pRTree->insertData_impl(
				reinsertlen[cIndex], reinsertdata[cIndex],
				*(reinsertmbr[cIndex]), reinsertid[cIndex],
				m_level, overflowTable);
		}

		delete[] reinsertdata;
		delete[] reinsertmbr;
		delete[] reinsertid;
		delete[] reinsertlen;

		return true;
	}
	else
	{
		NodePtr n;
		NodePtr nn;
		split(dataLength, pData, mbr, id, n, nn);

		if (pathBuffer.empty())
		{
			n->m_level = m_level;
			nn->m_level = m_level;
			n->m_identifier = -1;
			nn->m_identifier = -1;
			m_pRTree->writeNode(n.get());
			m_pRTree->writeNode(nn.get());

			NodePtr ptrR = m_pRTree->m_indexPool.acquire();
			if (ptrR.get() == 0)
			{
				ptrR = NodePtr(new Index(m_pRTree, m_pRTree->m_rootID, m_level + 1), &(m_pRTree->m_indexPool));
			}
			else
			{
				//ptrR->m_pRTree = m_pRTree;
				ptrR->m_identifier = m_pRTree->m_rootID;
				ptrR->m_level = m_level + 1;
				ptrR->m_nodeMBR = m_pRTree->m_infiniteRect;
			}

			ptrR->insertEntry(0, 0, n->m_nodeMBR, n->m_identifier);
			ptrR->insertEntry(0, 0, nn->m_nodeMBR, nn->m_identifier);

			m_pRTree->writeNode(ptrR.get());

			m_pRTree->m_stats.m_nodesInLevel[m_level] = 2;
			m_pRTree->m_stats.m_nodesInLevel.push_back(1);
			m_pRTree->m_stats.m_treeHeight = m_level + 2;
		}
		else
		{
			n->m_level = m_level;
			nn->m_level = m_level;
			n->m_identifier = m_identifier;
			nn->m_identifier = -1;

			m_pRTree->writeNode(n.get());
			m_pRTree->writeNode(nn.get());

			id_type cParent = pathBuffer.top(); pathBuffer.pop();
			NodePtr ptrN = m_pRTree->readNode(cParent);
			Index* p = static_cast<Index*>(ptrN.get());
			p->adjustTree(n.get(), nn.get(), pathBuffer, overflowTable);
		}

		return true;
	}
}

void Node::reinsertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& reinsert, std::vector<size_t>& keep)
{
	ReinsertEntry** v = new ReinsertEntry*[m_capacity + 1];

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = m_pRTree->m_rectPool.acquire();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	PointPtr nc = m_pRTree->m_pointPool.acquire();
	m_nodeMBR.getCenter(*nc);
	PointPtr c = m_pRTree->m_pointPool.acquire();

	for (size_t cChild = 0; cChild < m_capacity + 1; cChild++)
	{
		try
		{
			v[cChild] = new ReinsertEntry(cChild, 0.0);
		}
		catch (...)
		{
			for (size_t i = 0; i < cChild; i++) delete v[i];
			delete[] v;
			throw;
		}

		m_ptrMBR[cChild]->getCenter(*c);

		// calculate relative distance of every entry from the node MBR (ignore square root.)
		for (size_t cDim = 0; cDim < m_nodeMBR.DIMENSION; cDim++)
		{
			double d = nc->m_pCoords[cDim] - c->m_pCoords[cDim];
			v[cChild]->m_dist += d * d;
		}
	}

	// sort by increasing order of distances.
	::qsort(v, m_capacity + 1, sizeof(ReinsertEntry*), ReinsertEntry::compareReinsertEntry);

	size_t cReinsert = static_cast<size_t>(std::floor((m_capacity + 1) * m_pRTree->m_reinsertFactor));

	size_t cCount;

	for (cCount = 0; cCount < cReinsert; cCount++)
	{
		reinsert.push_back(v[cCount]->m_index);
		delete v[cCount];
	}

	for (cCount = cReinsert; cCount < m_capacity + 1; cCount++)
	{
		keep.push_back(v[cCount]->m_index);
		delete v[cCount];
	}

	delete[] v;
}

void Node::rtreeSplit(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& group1, std::vector<size_t>& group2)
{
	size_t cChild;
	size_t minimumLoad = static_cast<size_t>(std::floor(m_capacity * m_pRTree->m_fillFactor));

	// use this mask array for marking visited entries.
	byte* mask = new byte[m_capacity + 1];
	bzero(mask, m_capacity + 1);

	// insert new data in the node for easier manipulation. Data arrays are always
	// by one larger than node capacity.
	m_pDataLength[m_capacity] = dataLength;
	m_pData[m_capacity] = pData;
	m_ptrMBR[m_capacity] = m_pRTree->m_rectPool.acquire();
	*(m_ptrMBR[m_capacity]) = mbr;
	m_pIdentifier[m_capacity] = id;
	// m_totalDataLength does not need to be increased here.

	// initialize each group with the seed entries.
	size_t seed1, seed2;
	pickSeeds(seed1, seed2);

	group1.push_back(seed1);
	group2.push_back(seed2);

	mask[seed1] = 1;
	mask[seed2] = 1;

	// find MBR of each group.
	RectPtr mbr1 = m_pRTree->m_rectPool.acquire();
	*mbr1 = *(m_ptrMBR[seed1]);
	RectPtr mbr2 = m_pRTree->m_rectPool.acquire();
	*mbr2 = *(m_ptrMBR[seed2]);

	// count how many entries are left unchecked (exclude the seeds here.)
	size_t cRemaining = m_capacity + 1 - 2;

	while (cRemaining > 0)
	{
		if (minimumLoad - group1.size() == cRemaining)
		{
			// all remaining entries must be assigned to group1 to comply with minimun load requirement.
			for (cChild = 0; cChild < m_capacity + 1; cChild++)
			{
				if (mask[cChild] == 0)
				{
					group1.push_back(cChild);
					mask[cChild] = 1;
					cRemaining--;
				}
			}
		}
		else if (minimumLoad - group2.size() == cRemaining)
		{
			// all remaining entries must be assigned to group2 to comply with minimun load requirement.
			for (cChild = 0; cChild < m_capacity + 1; cChild++)
			{
				if (mask[cChild] == 0)
				{
					group2.push_back(cChild);
					mask[cChild] = 1;
					cRemaining--;
				}
			}
		}
		else
		{
			// For all remaining entries compute the difference of the cost of grouping an
			// entry in either group. When done, choose the entry that yielded the maximum
			// difference. In case of linear split, select any entry (e.g. the first one.)
			size_t sel;
			double md1 = 0.0, md2 = 0.0;
			double m = -Tools::DOUBLE_MAX;
			double d1, d2, d;
			double a1 = mbr1->getArea();
			double a2 = mbr2->getArea();

			RectPtr a = m_pRTree->m_rectPool.acquire();
			RectPtr b = m_pRTree->m_rectPool.acquire();

			for (cChild = 0; cChild < m_capacity + 1; cChild++)
			{
				if (mask[cChild] == 0)
				{
					mbr1->getCombinedRect(*a, *(m_ptrMBR[cChild]));
					d1 = a->getArea() - a1;
					mbr2->getCombinedRect(*b, *(m_ptrMBR[cChild]));
					d2 = b->getArea() - a2;
					d = std::abs(d1 - d2);

					if (d > m)
					{
						m = d;
						md1 = d1; md2 = d2;
						sel = cChild;
						if (m_pRTree->m_treeVariant== RV_LINEAR || m_pRTree->m_treeVariant == RV_RSTAR) break;
					}
				}
			}

			// determine the group where we should add the new entry.
			int32_t group = -1;

			if (md1 < md2)
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (md2 < md1)
			{
				group2.push_back(sel);
				group = 2;
			}
			else if (a1 < a2)
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (a2 < a1)
			{
				group2.push_back(sel);
				group = 2;
			}
			else if (group1.size() < group2.size())
			{
				group1.push_back(sel);
				group = 1;
			}
			else if (group2.size() < group1.size())
			{
				group2.push_back(sel);
				group = 2;
			}
			else
			{
				group1.push_back(sel);
				group = 1;
			}
			mask[sel] = 1;
			cRemaining--;
			if (group == 1)
			{
				mbr1->combineRect(*(m_ptrMBR[sel]));
			}
			else
			{
				mbr2->combineRect(*(m_ptrMBR[sel]));
			}
		}
	}

	delete[] mask;
}

void Node::rstarSplit(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& group1, std::vector<size_t>& group2)
{
	RstarSplitEntry** dataLow = 0;
	RstarSplitEntry** dataHigh = 0;

	try
	{
		dataLow = new RstarSplitEntry*[m_capacity + 1];
		dataHigh = new RstarSplitEntry*[m_capacity + 1];
	}
	catch (...)
	{
		delete[] dataLow;
		throw;
	}

	m_pDataLength[m_capacity] = dataLength;
	m_pData[m_capacity] = pData;
	m_ptrMBR[m_capacity] = m_pRTree->m_rectPool.acquire();
	*(m_ptrMBR[m_capacity]) = mbr;
	m_pIdentifier[m_capacity] = id;
	// m_totalDataLength does not need to be increased here.

	size_t nodeSPF = static_cast<size_t>(
		std::floor((m_capacity + 1) * m_pRTree->m_splitDistributionFactor));
	size_t splitDistribution = (m_capacity + 1) - (2 * nodeSPF) + 2;

	size_t cChild = 0, cDim, cIndex;

	for (cChild = 0; cChild <= m_capacity; cChild++)
	{
		try
		{
			dataLow[cChild] = new RstarSplitEntry(m_ptrMBR[cChild].get(), cChild, 0);
		}
		catch (...)
		{
			for (size_t i = 0; i < cChild; i++) delete dataLow[i];
			delete[] dataLow;
			delete[] dataHigh;
			throw;
		}

		dataHigh[cChild] = dataLow[cChild];
	}

	double minimumMargin = Tools::DOUBLE_MAX;
	size_t splitAxis = Tools::UINT32_MAX;
	size_t sortOrder = Tools::UINT32_MAX;

	// chooseSplitAxis.
	for (cDim = 0; cDim < m_pRTree->DIMENSION; cDim++)
	{
		::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareLow);
		::qsort(dataHigh, m_capacity + 1, sizeof(RstarSplitEntry*), RstarSplitEntry::compareHigh);

		// calculate sum of margins and overlap for all distributions.
		double marginl = 0.0;
		double marginh = 0.0;

		Rect bbl1, bbl2, bbh1, bbh2;

		for (cChild = 1; cChild <= splitDistribution; cChild++)
		{
			size_t l = nodeSPF - 1 + cChild;

			bbl1 = *(dataLow[0]->m_pRect);
			bbh1 = *(dataHigh[0]->m_pRect);

			for (cIndex = 1; cIndex < l; cIndex++)
			{
				bbl1.combineRect(*(dataLow[cIndex]->m_pRect));
				bbh1.combineRect(*(dataHigh[cIndex]->m_pRect));
			}

			bbl2 = *(dataLow[l]->m_pRect);
			bbh2 = *(dataHigh[l]->m_pRect);

			for (cIndex = l + 1; cIndex <= m_capacity; cIndex++)
			{
				bbl2.combineRect(*(dataLow[cIndex]->m_pRect));
				bbh2.combineRect(*(dataHigh[cIndex]->m_pRect));
			}

			marginl += bbl1.getMargin() + bbl2.getMargin();
			marginh += bbh1.getMargin() + bbh2.getMargin();
		} // for (cChild)

		double margin = std::min(marginl, marginh);

		// keep minimum margin as split axis.
		if (margin < minimumMargin)
		{
			minimumMargin = margin;
			splitAxis = cDim;
			sortOrder = (marginl < marginh) ? 0 : 1;
		}

		// increase the dimension according to which the data entries should be sorted.
		for (cChild = 0; cChild <= m_capacity; cChild++)
		{
			dataLow[cChild]->m_sortDim = cDim + 1;
		}
	} // for (cDim)

	for (cChild = 0; cChild <= m_capacity; cChild++)
	{
		dataLow[cChild]->m_sortDim = splitAxis;
	}

	::qsort(dataLow, m_capacity + 1, sizeof(RstarSplitEntry*), (sortOrder == 0) ? RstarSplitEntry::compareLow : RstarSplitEntry::compareHigh);

	double ma = Tools::DOUBLE_MAX;
	double mo = Tools::DOUBLE_MAX;
	size_t splitPoint = Tools::UINT32_MAX;

	Rect bb1, bb2;

	for (cChild = 1; cChild <= splitDistribution; cChild++)
	{
		size_t l = nodeSPF - 1 + cChild;

		bb1 = *(dataLow[0]->m_pRect);

		for (cIndex = 1; cIndex < l; cIndex++)
		{
			bb1.combineRect(*(dataLow[cIndex]->m_pRect));
		}

		bb2 = *(dataLow[l]->m_pRect);

		for (cIndex = l + 1; cIndex <= m_capacity; cIndex++)
		{
			bb2.combineRect(*(dataLow[cIndex]->m_pRect));
		}

		double o = bb1.getIntersectingArea(bb2);

		if (o < mo)
		{
			splitPoint = cChild;
			mo = o;
			ma = bb1.getArea() + bb2.getArea();
		}
		else if (o == mo)
		{
			double a = bb1.getArea() + bb2.getArea();

			if (a < ma)
			{
				splitPoint = cChild;
				ma = a;
			}
		}
	} // for (cChild)

	size_t l1 = nodeSPF - 1 + splitPoint;

	for (cIndex = 0; cIndex < l1; cIndex++)
	{
		group1.push_back(dataLow[cIndex]->m_index);
		delete dataLow[cIndex];
	}

	for (cIndex = l1; cIndex <= m_capacity; cIndex++)
	{
		group2.push_back(dataLow[cIndex]->m_index);
		delete dataLow[cIndex];
	}

	delete[] dataLow;
	delete[] dataHigh;
}

void Node::pickSeeds(size_t& index1, size_t& index2)
{
	double separation = -Tools::DOUBLE_MAX;
	double inefficiency = -Tools::DOUBLE_MAX;
	size_t cDim, cChild, cIndex;

	switch (m_pRTree->m_treeVariant)
	{
	case RV_LINEAR:
	case RV_RSTAR:
		for (cDim = 0; cDim < m_pRTree->DIMENSION; cDim++)
		{
			double leastLower = m_ptrMBR[0]->m_pLow[cDim];
			double greatestUpper = m_ptrMBR[0]->m_pHigh[cDim];
			size_t greatestLower = 0;
			size_t leastUpper = 0;
			double width;

			for (cChild = 1; cChild <= m_capacity; cChild++)
			{
				if (m_ptrMBR[cChild]->m_pLow[cDim] > m_ptrMBR[greatestLower]->m_pLow[cDim]) greatestLower = cChild;
				if (m_ptrMBR[cChild]->m_pHigh[cDim] < m_ptrMBR[leastUpper]->m_pHigh[cDim]) leastUpper = cChild;

				leastLower = std::min(m_ptrMBR[cChild]->m_pLow[cDim], leastLower);
				greatestUpper = std::max(m_ptrMBR[cChild]->m_pHigh[cDim], greatestUpper);
			}

			width = greatestUpper - leastLower;
			if (width <= 0) width = 1;

			double f = (m_ptrMBR[greatestLower]->m_pLow[cDim] - m_ptrMBR[leastUpper]->m_pHigh[cDim]) / width;

			if (f > separation)
			{
				index1 = leastUpper;
				index2 = greatestLower;
				separation = f;
			}
		}  // for (cDim)

		if (index1 == index2)
		{
			if (index2 == 0) index2++;
			else index2--;
		}

		break;
	case RV_QUADRATIC:
		// for each pair of Rects (account for overflow Rect too!)
		for (cChild = 0; cChild < m_capacity; cChild++)
		{
			double a = m_ptrMBR[cChild]->getArea();

			for (cIndex = cChild + 1; cIndex <= m_capacity; cIndex++)
			{
				// get the combined MBR of those two entries.
				Rect r;
				m_ptrMBR[cChild]->getCombinedRect(r, *(m_ptrMBR[cIndex]));

				// find the inefficiency of grouping these entries together.
				double d = r.getArea() - a - m_ptrMBR[cIndex]->getArea();

				if (d > inefficiency)
				{
					inefficiency = d;
					index1 = cChild;
					index2 = cIndex;
				}
			}  // for (cIndex)
		} // for (cChild)

		break;
	default:
		throw Tools::NotSupportedException("Node::pickSeeds: Tree variant not supported.");
	}
}

void Node::condenseTree(std::stack<NodePtr>& toReinsert, std::stack<id_type>& pathBuffer, NodePtr& ptrThis)
{
	size_t minimumLoad = static_cast<size_t>(std::floor(m_capacity * m_pRTree->m_fillFactor));

	if (pathBuffer.empty())
	{
		// eliminate root if it has only one child.
		if (m_level != 0 && m_children == 1)
		{
			NodePtr ptrN = m_pRTree->readNode(m_pIdentifier[0]);
			m_pRTree->deleteNode(ptrN.get());
			ptrN->m_identifier = m_pRTree->m_rootID;
			m_pRTree->writeNode(ptrN.get());

			m_pRTree->m_stats.m_nodesInLevel.pop_back();
			m_pRTree->m_stats.m_treeHeight -= 1;
			// HACK: pending deleteNode for deleted child will decrease nodesInLevel, later on.
			m_pRTree->m_stats.m_nodesInLevel[m_pRTree->m_stats.m_treeHeight - 1] = 2;
		}
	}
	else
	{
		id_type cParent = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrParent = m_pRTree->readNode(cParent);
		Index* p = static_cast<Index*>(ptrParent.get());

		// find the entry in the parent, that points to this node.
		size_t child;

		for (child = 0; child != p->m_children; child++)
		{
			if (p->m_pIdentifier[child] == m_identifier) break;
		}

		if (m_children < minimumLoad)
		{
			// used space less than the minimum
			// 1. eliminate node entry from the parent. deleteEntry will fix the parent's MBR.
			p->deleteEntry(child);
			// 2. add this node to the stack in order to reinsert its entries.
			toReinsert.push(ptrThis);
		}
		else
		{
			// adjust the entry in 'p' to contain the new bounding region of this node.
			*(p->m_ptrMBR[child]) = m_nodeMBR;

			// global recalculation necessary since the MBR can only shrink in size,
			// due to data removal.
			if (m_pRTree->m_bTightMBRs)
			{
				for (size_t cDim = 0; cDim < p->m_nodeMBR.DIMENSION; cDim++)
				{
					p->m_nodeMBR.m_pLow[cDim] = Tools::DOUBLE_MAX;
					p->m_nodeMBR.m_pHigh[cDim] = -Tools::DOUBLE_MAX;

					for (size_t cChild = 0; cChild < p->m_children; cChild++)
					{
						p->m_nodeMBR.m_pLow[cDim] = std::min(p->m_nodeMBR.m_pLow[cDim], p->m_ptrMBR[cChild]->m_pLow[cDim]);
						p->m_nodeMBR.m_pHigh[cDim] = std::max(p->m_nodeMBR.m_pHigh[cDim], p->m_ptrMBR[cChild]->m_pHigh[cDim]);
					}
				}
			}
		}

		// write parent node back to storage.
		m_pRTree->writeNode(p);

		p->condenseTree(toReinsert, pathBuffer, ptrParent);
	}
}

NodePtr Node::chooseSubtree(const Rect& mbr, size_t level, std::stack<id_type>& pathBuffer)
{
	throw Tools::IllegalStateException("Node::chooseSubtree: should never be called. ");
}

NodePtr Node::findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer)
{
	throw Tools::IllegalStateException("Node::findLeaf: should never be called. ");
}

void Node::split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& left, NodePtr& right)
{
	throw Tools::IllegalStateException("Node::split: should never be called. ");
}

RectPtr Node::getRectPtr() const
{
	return m_pRTree->m_rectPool.acquire();
}