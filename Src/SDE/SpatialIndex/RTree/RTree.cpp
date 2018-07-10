#include "RTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "BulkLoader.h"
#include "../Utility/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::RTree;

ISpatialIndex* IS_SDE::SpatialIndex::RTree::returnRTree(IStorageManager& sm, Tools::PropertySet& ps)
{
	ISpatialIndex* si = new SpatialIndex::RTree::RTree(sm, ps);
	return si;
}

ISpatialIndex* IS_SDE::SpatialIndex::RTree::createNewRTree(
	IStorageManager& sm,
	double fillFactor,
	size_t indexCapacity,
	size_t leafCapacity,
	RTreeVariant rv,
	id_type& indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = fillFactor;
	ps.setProperty("FillFactor", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = indexCapacity;
	ps.setProperty("IndexCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = leafCapacity;
	ps.setProperty("LeafCapacity", var);

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = rv;
	ps.setProperty("TreeVariant", var);

	ISpatialIndex* ret = returnRTree(sm, ps);

	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.lVal;

	return ret;
}

ISpatialIndex* IS_SDE::SpatialIndex::RTree::createAndBulkLoadNewRTree(
	BulkLoadMethod m,
	IDataStream& stream,
	IStorageManager& sm,
	double fillFactor,
	size_t indexCapacity,
	size_t leafCapacity,
	RTreeVariant rv,
	id_type& indexIdentifier)
{
	ISpatialIndex* tree = createNewRTree(sm, fillFactor, indexCapacity, leafCapacity, rv, indexIdentifier);

	size_t bindex = static_cast<size_t>(std::floor(static_cast<double>(indexCapacity * fillFactor)));
	size_t bleaf = static_cast<size_t>(std::floor(static_cast<double>(leafCapacity * fillFactor)));

	BulkLoader bl;

	switch (m)
	{
	case BLM_STR:
		//FIXME: find available memory here.
		bl.bulkLoadUsingSTR(static_cast<RTree*>(tree), stream, bindex, bleaf, 200000);
		break;
	default:
		throw Tools::IllegalArgumentException("createAndBulkLoadNewRTree: Unknown bulk load method.");
		break;
	}

	return tree;
}

ISpatialIndex* IS_SDE::SpatialIndex::RTree::loadRTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = static_cast<int32_t>(indexIdentifier);
	ps.setProperty("IndexIdentifier", var);

	return returnRTree(sm, ps);
}

IS_SDE::SpatialIndex::RTree::RTree::RTree(IStorageManager& sm, Tools::PropertySet& ps) :
	BaseTree(sm, ps, 100, 100, StorageManager::NewPage),
	m_treeVariant(RV_RSTAR),
	m_fillFactor(0.7),
	m_nearMinimumOverlapFactor(32),
	m_splitDistributionFactor(0.4),
	m_reinsertFactor(0.3),
	m_bTightMBRs(true)
{
#ifdef PTHREADS
	pthread_rwlock_init(&m_rwLock, NULL);
#else
	m_rwLock = false;
#endif

	Tools::Variant var = ps.getProperty("IndexIdentifier");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG) throw Tools::IllegalArgumentException("RTree: Property IndexIdentifier must be Tools::VT_LONG");
		m_headerID = var.m_val.lVal;
		initOld(ps);
	}
	else
	{
		initNew(ps);
		var.m_varType = Tools::VT_LONG;
		var.m_val.lVal = static_cast<int32_t>(m_headerID);
		ps.setProperty("IndexIdentifier", var);
	}
}

IS_SDE::SpatialIndex::RTree::RTree::~RTree()
{
#ifdef PTHREADS
	pthread_rwlock_destroy(&m_rwLock);
#endif

	setFirstDisplayScope();

	storeHeader();
}

//
// ISpatialIndex interface
//

void IS_SDE::SpatialIndex::RTree::RTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc)
{
#ifdef PTHREADS
	Tools::SharedLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("nearestNeighborQuery: cannot acquire a shared lock");
#endif

	try
	{
		std::priority_queue<NNEntry*, std::vector<NNEntry*>, NNEntry::ascending> queue;

		queue.push(new NNEntry(m_rootID, 0, 0.0));

		size_t count = 0;
		double knearest = 0.0;

		while (! queue.empty())
		{
			NNEntry* pFirst = queue.top();

			// report all nearest neighbors with equal greatest distances.
			// (neighbors can be more than k, if many happen to have the same greatest distance).
			if (count >= k && pFirst->m_minDist > knearest)	break;

			queue.pop();

			if (pFirst->m_pEntry == 0)
			{
				// n is a leaf or an index.
				NodePtr n = readNode(pFirst->m_id);
				v.visitNode(*n);

				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (n->m_level == 0)
					{
						Data* e = new Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
						// we need to compare the query with the actual data entry here, so we call the
						// appropriate getMinimumDistance method of NearestNeighborComparator.
						queue.push(new NNEntry(n->m_pIdentifier[cChild], e, nnc.getMinimumDistance(query, *e)));
					}
					else
					{
						queue.push(new NNEntry(n->m_pIdentifier[cChild], 0, nnc.getMinimumDistance(query, *(n->m_ptrMBR[cChild]))));
					}
				}
			}
			else
			{
				v.visitData(*(static_cast<IData*>(pFirst->m_pEntry)), Tools::UINT32_MAX);
				m_stats.m_queryResults++;
				count++;
				knearest = pFirst->m_minDist;
				delete pFirst->m_pEntry;
			}

			delete pFirst;
		}

		while (! queue.empty())
		{
			NNEntry* e = queue.top(); queue.pop();
			if (e->m_pEntry != 0) delete e->m_pEntry;
			delete e;
		}

#ifndef PTHREADS
		m_rwLock = false;
#endif
	}
	catch (...)
	{
#ifndef PTHREADS
		m_rwLock = false;
#endif
		throw;
	}
}

void IS_SDE::SpatialIndex::RTree::RTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	NNComparator nnc;
	nearestNeighborQuery(k, query, v, nnc);
}

void IS_SDE::SpatialIndex::RTree::RTree::selfJoinQuery(const IShape& query, IVisitor& v)
{
#ifdef PTHREADS
	Tools::SharedLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("selfJoinQuery: cannot acquire a shared lock");
#endif

	try
	{
		RectPtr mbr = m_rectPool.acquire();
		query.getMBR(*mbr);
		selfJoinQuery(m_rootID, m_rootID, *mbr, v);

#ifndef PTHREADS
		m_rwLock = false;
#endif
	}
	catch (...)
	{
#ifndef PTHREADS
		m_rwLock = false;
#endif
		throw;
	}
}

bool IS_SDE::SpatialIndex::RTree::RTree::countIntersectsQuery(const IShape& query, ICountVisitor& v)
{
	throw Tools::NotSupportedException("Not support.");
}

void IS_SDE::SpatialIndex::RTree::RTree::getIndexProperties(Tools::PropertySet& out) const
{
	Tools::Variant var;

	// index capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_indexCapacity;
	out.setProperty("IndexCapacity", var);

	// leaf capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_leafCapacity;
	out.setProperty("LeafCapacity", var);

	// R-tree variant
	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = m_treeVariant;
	out.setProperty("TreeVariant", var);

	// fill factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_fillFactor;
	out.setProperty("FillFactor", var);

	// near minimum overlap factor
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_nearMinimumOverlapFactor;
	out.setProperty("NearMinimumOverlapFactor", var);

	// split distribution factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_splitDistributionFactor;
	out.setProperty("SplitDistributionFactor", var);

	// reinsert factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_reinsertFactor;
	out.setProperty("ReinsertFactor", var);

	// tight MBRs
	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = m_bTightMBRs;
	out.setProperty("EnsureTightMBRs", var);

	// index pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_indexPool.getCapacity();
	out.setProperty("IndexPoolCapacity", var);

	// leaf pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_leafPool.getCapacity();
	out.setProperty("LeafPoolCapacity", var);

	// rect pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_rectPool.getCapacity();
	out.setProperty("RectPoolCapacity", var);

	// point pool capacity
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = m_pointPool.getCapacity();
	out.setProperty("PointPoolCapacity", var);
}

bool IS_SDE::SpatialIndex::RTree::RTree::isIndexValid()
{
	bool ret = true;
	std::stack<ValidateEntry> st;
	NodePtr root = readNode(m_rootID);

	if (root->m_level != m_stats.m_treeHeight - 1)
	{
		std::cerr << "Invalid tree height." << std::endl;
		return false;
	}

	std::map<size_t, size_t> nodesInLevel;
	nodesInLevel.insert(std::pair<size_t, size_t>(root->m_level, 1));

	ValidateEntry e(root->m_nodeMBR, root);
	st.push(e);

	while (! st.empty())
	{
		e = st.top(); st.pop();

		Rect tmpRect;
		tmpRect = m_infiniteRect;

		for (size_t cDim = 0; cDim < tmpRect.DIMENSION; cDim++)
		{
			tmpRect.m_pLow[cDim] = Tools::DOUBLE_MAX;
			tmpRect.m_pHigh[cDim] = -Tools::DOUBLE_MAX;

			for (size_t cChild = 0; cChild < e.m_pNode->m_children; cChild++)
			{
				tmpRect.m_pLow[cDim] = std::min(tmpRect.m_pLow[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pLow[cDim]);
				tmpRect.m_pHigh[cDim] = std::max(tmpRect.m_pHigh[cDim], e.m_pNode->m_ptrMBR[cChild]->m_pHigh[cDim]);
			}
		}

		if (! (tmpRect == e.m_pNode->m_nodeMBR))
		{
			std::cerr << "Invalid parent information." << std::endl;
			ret = false;
		}
		else if (! (tmpRect == e.m_parentMBR))
		{
			std::cerr << "Error in parent." << std::endl;
			ret = false;
		}

		if (e.m_pNode->m_level != 0)
		{
			for (size_t cChild = 0; cChild < e.m_pNode->m_children; cChild++)
			{
				NodePtr ptrN = readNode(e.m_pNode->m_pIdentifier[cChild]);
				ValidateEntry tmpEntry(*(e.m_pNode->m_ptrMBR[cChild]), ptrN);

				std::map<size_t, size_t>::iterator itNodes = nodesInLevel.find(tmpEntry.m_pNode->m_level);

				if (itNodes == nodesInLevel.end())
				{
					nodesInLevel.insert(std::pair<size_t, size_t>(tmpEntry.m_pNode->m_level, 1l));
				}
				else
				{
					nodesInLevel[tmpEntry.m_pNode->m_level] = nodesInLevel[tmpEntry.m_pNode->m_level] + 1;
				}

				st.push(tmpEntry);
			}
		}
	}

	size_t nodes = 0;
	for (size_t cLevel = 0; cLevel < m_stats.m_treeHeight; cLevel++)
	{
		if (nodesInLevel[cLevel] != m_stats.m_nodesInLevel[cLevel])
		{
			std::cerr << "Invalid nodesInLevel information." << std::endl;
			ret = false;
		}

		nodes += m_stats.m_nodesInLevel[cLevel];
	}

	if (nodes != m_stats.m_nodes)
	{
		std::cerr << "Invalid number of nodes information." << std::endl;
		ret = false;
	}

	return ret;
}

void IS_SDE::SpatialIndex::RTree::RTree::getStatistics(IStatistics** out) const
{
	*out = new Statistics(m_stats);
}

//void IS_SDE::SpatialIndex::RTree::RTree::checkStoreState(ICheckStoreStrategy& css)
//{
//	id_type next = m_rootID;
//	bool hasNext = true;
//
//	while (hasNext)
//	{
//		NodePtr n = readNode(next);
//		css.getNextEntry(m_pStorageManager, *n, next, hasNext);
//	}
//}

void IS_SDE::SpatialIndex::RTree::RTree::initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_LONG ||
			(var.m_val.lVal != RV_LINEAR &&
			var.m_val.lVal != RV_QUADRATIC &&
			var.m_val.lVal != RV_RSTAR))
			throw Tools::IllegalArgumentException("initNew: Property TreeVariant must be Tools::VT_LONG and of RTreeVariant type");

		m_treeVariant = static_cast<RTreeVariant>(var.m_val.lVal);
	}

	// fill factor
	// it cannot be larger than 50%, since linear and quadratic split algorithms
	// require assigning to both nodes the same number of entries.
	var = ps.getProperty("FillFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			((m_treeVariant == RV_LINEAR || m_treeVariant == RV_QUADRATIC) && var.m_val.dblVal > 0.5) ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property FillFactor must be Tools::VT_DOUBLE and in (0.0, 1.0) for RSTAR, (0.0, 0.5) for LINEAR and QUADRATIC");

		m_fillFactor = var.m_val.dblVal;
	}

	// index capacity
	var = ps.getProperty("IndexCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 4)
			throw Tools::IllegalArgumentException("initNew: Property IndexCapacity must be Tools::VT_ULONG and >= 4");

		m_indexCapacity = var.m_val.ulVal;
	}

	// leaf capacity
	var = ps.getProperty("LeafCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 4)
			throw Tools::IllegalArgumentException("initNew: Property LeafCapacity must be Tools::VT_ULONG and >= 4");

		m_leafCapacity = var.m_val.ulVal;
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_ULONG ||
			var.m_val.ulVal < 1 ||
			var.m_val.ulVal > m_indexCapacity ||
			var.m_val.ulVal > m_leafCapacity)
			throw Tools::IllegalArgumentException("initNew: Property NearMinimumOverlapFactor must be Tools::VT_ULONG and less than both index and leaf capacities");

		m_nearMinimumOverlapFactor = var.m_val.ulVal;
	}

	// split distribution factor
	var = ps.getProperty("SplitDistributionFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property SplitDistributionFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_splitDistributionFactor = var.m_val.dblVal;
	}

	// reinsert factor
	var = ps.getProperty("ReinsertFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal <= 0.0 ||
			var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initNew: Property ReinsertFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_reinsertFactor = var.m_val.dblVal;
	}

	// tight MBRs
	var = ps.getProperty("EnsureTightMBRs");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL)
			throw Tools::IllegalArgumentException("initNew: Property EnsureTightMBRs must be Tools::VT_BOOL");

		m_bTightMBRs = var.m_val.blVal;
	}

	// index pool capacity
	var = ps.getProperty("IndexPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property IndexPoolCapacity must be Tools::VT_ULONG");

		m_indexPool.setCapacity(var.m_val.ulVal);
	}

	// leaf pool capacity
	var = ps.getProperty("LeafPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property LeafPoolCapacity must be Tools::VT_ULONG");

		m_leafPool.setCapacity(var.m_val.ulVal);
	}

	// rect pool capacity
	var = ps.getProperty("RectPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property RectPoolCapacity must be Tools::VT_ULONG");

		m_rectPool.setCapacity(var.m_val.ulVal);
	}

	// point pool capacity
	var = ps.getProperty("PointPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG)
			throw Tools::IllegalArgumentException("initNew: Property PointPoolCapacity must be Tools::VT_ULONG");

		m_pointPool.setCapacity(var.m_val.ulVal);
	}

	m_infiniteRect.makeInfinite();

	m_stats.m_treeHeight = 1;
	m_stats.m_nodesInLevel.push_back(0);

	Leaf root(this, -1);
	m_rootID = writeNode(&root);

	storeHeader();
}

void IS_SDE::SpatialIndex::RTree::RTree::initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

	// tree variant
	var = ps.getProperty("TreeVariant");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (
			var.m_varType != Tools::VT_LONG ||
			(var.m_val.lVal != RV_LINEAR &&
			var.m_val.lVal != RV_QUADRATIC &&
			var.m_val.lVal != RV_RSTAR))
			throw Tools::IllegalArgumentException("initOld: Property TreeVariant must be Tools::VT_LONG and of RTreeVariant type");

		m_treeVariant = static_cast<RTreeVariant>(var.m_val.lVal);
	}

	// near minimum overlap factor
	var = ps.getProperty("NearMinimumOverlapFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (	
			var.m_varType != Tools::VT_ULONG ||
			var.m_val.ulVal < 1 ||
			var.m_val.ulVal > m_indexCapacity ||
			var.m_val.ulVal > m_leafCapacity)
			throw Tools::IllegalArgumentException("initOld: Property NearMinimumOverlapFactor must be Tools::VT_ULONG and less than both index and leaf capacities");

		m_nearMinimumOverlapFactor = var.m_val.ulVal;
	}

	// split distribution factor
	var = ps.getProperty("SplitDistributionFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initOld: Property SplitDistributionFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_splitDistributionFactor = var.m_val.dblVal;
	}

	// reinsert factor
	var = ps.getProperty("ReinsertFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE || var.m_val.dblVal <= 0.0 || var.m_val.dblVal >= 1.0)
			throw Tools::IllegalArgumentException("initOld: Property ReinsertFactor must be Tools::VT_DOUBLE and in (0.0, 1.0)");

		m_reinsertFactor = var.m_val.dblVal;
	}

	// tight MBRs
	var = ps.getProperty("EnsureTightMBRs");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL) throw Tools::IllegalArgumentException("initOld: Property EnsureTightMBRs must be Tools::VT_BOOL");

		m_bTightMBRs = var.m_val.blVal;
	}

	// index pool capacity
	var = ps.getProperty("IndexPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property IndexPoolCapacity must be Tools::VT_ULONG");

		m_indexPool.setCapacity(var.m_val.ulVal);
	}

	// leaf pool capacity
	var = ps.getProperty("LeafPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property LeafPoolCapacity must be Tools::VT_ULONG");

		m_leafPool.setCapacity(var.m_val.ulVal);
	}

	// rect pool capacity
	var = ps.getProperty("RectPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property RectPoolCapacity must be Tools::VT_ULONG");

		m_rectPool.setCapacity(var.m_val.ulVal);
	}

	// point pool capacity
	var = ps.getProperty("PointPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalArgumentException("initOld: Property PointPoolCapacity must be Tools::VT_ULONG");

		m_pointPool.setCapacity(var.m_val.ulVal);
	}

	m_infiniteRect.makeInfinite();
}

void IS_SDE::SpatialIndex::RTree::RTree::storeHeader()
{
	const size_t headerSize =
		sizeof(id_type) +						// m_rootID
		sizeof(id_type) +						// m_nextObjID
		sizeof(RTreeVariant) +					// m_treeVariant
		sizeof(double)	+                       // m_fillFactor
		sizeof(size_t)	+                       // m_indexCapacity
		sizeof(size_t)	+                       // m_leafCapacity
		sizeof(Rect)	+						// m_firstDisplayScope
		sizeof(size_t)	+                       // m_nearMinimumOverlapFactor
		sizeof(double)	+                       // m_splitDistributionFactor
		sizeof(double)	+                       // m_reinsertFactor
		sizeof(char)	+                       // m_bTightMBRs
		sizeof(size_t)	+                       // m_stats.m_nodes
		sizeof(size_t)	+                       // m_stats.m_data
		sizeof(size_t)	+                       // m_stats.m_treeHeight
		m_stats.m_treeHeight * sizeof(size_t);	// m_stats.m_nodesInLevel

	byte* header = new byte[headerSize];
	byte* ptr = header;

	memcpy(ptr, &m_rootID, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_nextObjID, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_treeVariant, sizeof(RTreeVariant));
	ptr += sizeof(RTreeVariant);
	memcpy(ptr, &m_fillFactor, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_indexCapacity, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_leafCapacity, sizeof(size_t));
	ptr += sizeof(size_t);

	size_t lenRect = m_firstDisplayScope.getByteArraySize();
	if (m_firstDisplayScope.isEmpty())
	{
		memset(ptr, 0, lenRect);
	}
	else
	{
		byte* pRect; size_t lenRect;
		m_firstDisplayScope.storeToByteArray(&pRect, lenRect);
		memcpy(ptr, pRect, lenRect);
		delete[] pRect;
	}
	ptr += lenRect;

	memcpy(ptr, &m_nearMinimumOverlapFactor, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_splitDistributionFactor, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_reinsertFactor, sizeof(double));
	ptr += sizeof(double);
	char c = (char) m_bTightMBRs;
	memcpy(ptr, &c, sizeof(char));
	ptr += sizeof(char);
	memcpy(ptr, &(m_stats.m_nodes), sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &(m_stats.m_data), sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &(m_stats.m_treeHeight), sizeof(size_t));
	ptr += sizeof(size_t);

	for (size_t cLevel = 0; cLevel < m_stats.m_treeHeight; cLevel++)
	{
		memcpy(ptr, &(m_stats.m_nodesInLevel[cLevel]), sizeof(size_t));
		ptr += sizeof(size_t);
	}

	m_pStorageManager->storeByteArray(m_headerID, headerSize, header);

	delete[] header;
}

void IS_SDE::SpatialIndex::RTree::RTree::loadHeader()
{
	size_t headerSize;
	byte* header = 0;
	m_pStorageManager->loadByteArray(m_headerID, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_rootID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_nextObjID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_treeVariant, ptr, sizeof(RTreeVariant));
	ptr += sizeof(RTreeVariant);
	memcpy(&m_fillFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_indexCapacity, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_leafCapacity, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	size_t rectSize = m_firstDisplayScope.getByteArraySize();
	byte* pRect = new byte[rectSize];
	memcpy(pRect, ptr, rectSize);
	m_firstDisplayScope.loadFromByteArray(pRect);
	delete pRect;
	ptr += rectSize;

	memcpy(&m_nearMinimumOverlapFactor, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_splitDistributionFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_reinsertFactor, ptr, sizeof(double));
	ptr += sizeof(double);
	char c;
	memcpy(&c, ptr, sizeof(char));
	m_bTightMBRs = (bool) c;
	ptr += sizeof(char);
	memcpy(&(m_stats.m_nodes), ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&(m_stats.m_data), ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&(m_stats.m_treeHeight), ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	for (size_t cLevel = 0; cLevel < m_stats.m_treeHeight; cLevel++)
	{
		size_t cNodes;
		memcpy(&cNodes, ptr, sizeof(size_t));
		ptr += sizeof(size_t);
		m_stats.m_nodesInLevel.push_back(cNodes);
	}

	delete[] header;
}

void IS_SDE::SpatialIndex::RTree::RTree::insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID)
{
	std::stack<id_type> pathBuffer;
	byte* overflowTable = 0;

	try
	{
		NodePtr root = readNode(m_rootID);

		overflowTable = new byte[root->m_level];
		bzero(overflowTable, root->m_level);

		NodePtr l = root->chooseSubtree(mbr, 0, pathBuffer);
		if (l.get() == root.get())
		{
			assert(root.unique());
			root.relinquish();
		}
		*nodeID = l->getIdentifier();
		l->insertData(dataLength, pData, mbr, objID, pathBuffer, overflowTable);

		delete[] overflowTable;
		m_stats.m_data++;
	}
	catch (...)
	{
		delete[] overflowTable;
		throw;
	}
}

void IS_SDE::SpatialIndex::RTree::RTree::insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type id, size_t level, byte* overflowTable)
{
	std::stack<id_type> pathBuffer;
	NodePtr root = readNode(m_rootID);
	NodePtr n = root->chooseSubtree(mbr, level, pathBuffer);

	assert(n->m_level == level);

	if (n.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}
	n->insertData(dataLength, pData, mbr, id, pathBuffer, overflowTable);
}

bool IS_SDE::SpatialIndex::RTree::RTree::deleteData_impl(const Rect& mbr, id_type id)
{
	std::stack<id_type> pathBuffer;
	NodePtr root = readNode(m_rootID);
	NodePtr l = root->findLeaf(mbr, id, pathBuffer);
	if (l.get() == root.get())
	{
		assert(root.unique());
		root.relinquish();
	}

	if (l.get() != 0)
	{
		Leaf* pL = static_cast<Leaf*>(l.get());
		pL->deleteData(id, pathBuffer);
		m_stats.m_data--;
		return true;
	}

	return false;
}

id_type IS_SDE::SpatialIndex::RTree::RTree::writeNode(Node* n)
{
	byte* buffer;
	size_t dataLength;
	n->storeToByteArray(&buffer, dataLength);

	id_type page;
	if (n->m_identifier < 0) 
		page = StorageManager::NewPage;
	else 
		page = n->m_identifier;

	try
	{
		m_pStorageManager->storeByteArray(page, dataLength, buffer);
		delete[] buffer;
	}
	catch (Tools::InvalidPageException& e)
	{
		delete[] buffer;
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("writeNode: failed with Tools::InvalidPageException");
	}

	if (n->m_identifier < 0)
	{
		n->m_identifier = page;
		m_stats.m_nodes++;

#ifndef NDEBUG
		try
		{
			m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel.at(n->m_level) + 1;
		}
		catch(...)
		{
			throw Tools::IllegalStateException("writeNode: writing past the end of m_nodesInLevel.");
		}
#else
		m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel[n->m_level] + 1;
#endif
	}

	m_stats.m_writes++;

	for (size_t cIndex = 0; cIndex < m_writeNodeCommands.size(); cIndex++)
	{
		m_writeNodeCommands[cIndex]->execute(*n);
	}

	return page;
}

NodePtr IS_SDE::SpatialIndex::RTree::RTree::readNode(id_type id)
{
	size_t dataLength;
	byte* buffer;

	try
	{
		m_pStorageManager->loadByteArray(id, dataLength, &buffer);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("readNode: failed with Tools::InvalidPageException");
	}

	try
	{
		size_t nodeType;
		memcpy(&nodeType, buffer, sizeof(size_t));

		NodePtr n;

		if (nodeType == PersistentIndex) 
			n = m_indexPool.acquire();
		else if (nodeType == PersistentLeaf) 
			n = m_leafPool.acquire();
		else 
			throw Tools::IllegalStateException("readNode: failed reading the correct node type information");

		if (n.get() == 0)
		{
			if (nodeType == PersistentIndex) n = NodePtr(new Index(this, -1, 0), &m_indexPool);
			else if (nodeType == PersistentLeaf) n = NodePtr(new Leaf(this, -1), &m_leafPool);
		}

		n->m_pRTree = this;
		n->m_identifier = id;
		n->loadFromByteArray(buffer);

		m_stats.m_reads++;

		for (size_t cIndex = 0; cIndex < m_readNodeCommands.size(); cIndex++)
		{
			m_readNodeCommands[cIndex]->execute(*n);
		}

		delete[] buffer;
		return n;
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}
}

void IS_SDE::SpatialIndex::RTree::RTree::deleteNode(Node* n)
{
	try
	{
		m_pStorageManager->deleteByteArray(n->m_identifier);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("deleteNode: failed with Tools::InvalidPageException");
	}

	m_stats.m_nodes--;
	m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel[n->m_level] - 1;

	for (size_t cIndex = 0; cIndex < m_deleteNodeCommands.size(); cIndex++)
	{
		m_deleteNodeCommands[cIndex]->execute(*n);
	}
}

void IS_SDE::SpatialIndex::RTree::RTree::rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v, bool isFine)
{
#ifdef PTHREADS
	Tools::SharedLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("rangeQuery: cannot acquire a shared lock");
#endif

	try
	{
		std::stack<NodePtr> st;

		NodePtr root = readNode(m_rootID);
		if (root->m_children > 0 && query.intersectsShape(root->m_nodeMBR)) 
			st.push(root);

		while (! st.empty())
		{
			NodePtr n = st.top(); st.pop();

			if (n->isLeaf())
			{
				v.visitNode(*n);

				IShape* nodeMBR;
				n->getShape(&nodeMBR);
				bool bNodeIn = query.containsShape(*nodeMBR);
				delete nodeMBR;

				if (bNodeIn)
				{
					v.visitData(*n);

					if (v.shouldCheckEachData())
					{
						for (size_t cChild = 0; cChild < n->m_children; cChild++)
						{
							Data data = Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
							v.visitData(data, cChild);
							m_stats.m_queryResults++;
						}
					}
				}
				else
				{
					for (size_t cChild = 0; cChild < n->m_children; cChild++)
					{
						bool b;

						if (bNodeIn)
						{
							b = true;
						}
						else
						{
							if (type == ContainmentQuery) 
							{
								b = query.containsShape(*(n->m_ptrMBR[cChild]));
								if (isFine && !b)
								{
									IShape* pS;
									Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]).getShape(&pS);
									b = query.containsShape(*pS);
									delete pS;
								}
							}
							else
							{
								b = query.intersectsShape(*(n->m_ptrMBR[cChild]));
								if (isFine && b)
								{
									IShape* pS;
									Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]).getShape(&pS);
									b = query.intersectsShape(*pS);
									delete pS;
								}
							}
						}

						if (b)
						{
							Data data = Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
							v.visitData(data, cChild);
							m_stats.m_queryResults++;
						}
					}
				}

			}
			else
			{
				v.visitNode(*n);

				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (query.intersectsShape(*(n->m_ptrMBR[cChild]))) st.push(readNode(n->m_pIdentifier[cChild]));
				}
			}
		}

#ifndef PTHREADS
		m_rwLock = false;
#endif
	}
	catch (...)
	{
#ifndef PTHREADS
		m_rwLock = false;
#endif
		throw;
	}
}

void IS_SDE::SpatialIndex::RTree::RTree::selfJoinQuery(id_type id1, id_type id2, const Rect& r, IVisitor& vis)
{
	NodePtr n1 = readNode(id1);
	NodePtr n2 = readNode(id2);
	vis.visitNode(*n1);
	vis.visitNode(*n2);

	for (size_t cChild1 = 0; cChild1 < n1->m_children; cChild1++)
	{
		if (r.intersectsRect(*(n1->m_ptrMBR[cChild1])))
		{
			for (size_t cChild2 = 0; cChild2 < n2->m_children; cChild2++)
			{
				if (
					r.intersectsRect(*(n2->m_ptrMBR[cChild2])) &&
					n1->m_ptrMBR[cChild1]->intersectsRect(*(n2->m_ptrMBR[cChild2])))
				{
					if (n1->m_level == 0)
					{
						if (n1->m_pIdentifier[cChild1] != n2->m_pIdentifier[cChild2])
						{
							assert(n2->m_level == 0);

							std::vector<const IData*> v;
							Data e1(n1->m_pDataLength[cChild1], n1->m_pData[cChild1], *(n1->m_ptrMBR[cChild1]), n1->m_pIdentifier[cChild1]);
							Data e2(n2->m_pDataLength[cChild2], n2->m_pData[cChild2], *(n2->m_ptrMBR[cChild2]), n2->m_pIdentifier[cChild2]);
							v.push_back(&e1);
							v.push_back(&e2);
							vis.visitData(v);
						}
					}
					else
					{
						Rect rr = r.getIntersectingRect(n1->m_ptrMBR[cChild1]->getIntersectingRect(*(n2->m_ptrMBR[cChild2])));
						selfJoinQuery(n1->m_pIdentifier[cChild1], n2->m_pIdentifier[cChild2], rr, vis);
					}
				}
			}
		}
	}
}

std::ostream& SpatialIndex::RTree::operator<<(std::ostream& os, const SpatialIndex::RTree::RTree& t)
{
	using std::endl;

	os	<< "Fill factor: " << t.m_fillFactor << endl
		<< "Index capacity: " << t.m_indexCapacity << endl
		<< "Leaf capacity: " << t.m_leafCapacity << endl
		<< "Tight MBRs: " << ((t.m_bTightMBRs) ? "enabled" : "disabled") << endl;

	if (t.m_treeVariant == RV_RSTAR)
	{
		os	<< "Near minimum overlap factor: " << t.m_nearMinimumOverlapFactor << endl
			<< "Reinsert factor: " << t.m_reinsertFactor << endl
			<< "Split distribution factor: " << t.m_splitDistributionFactor << endl;
	}

	if (t.m_stats.getNumberOfNodesInLevel(0) > 0)
		os	<< "Utilization: " << 100 * t.m_stats.getNumberOfData() / (t.m_stats.getNumberOfNodesInLevel(0) * t.m_leafCapacity) << "%" << endl
		<< t.m_stats;

#ifndef NDEBUG
	os	<< "Leaf pool hits: " << t.m_leafPool.m_hits << endl
		<< "Leaf pool misses: " << t.m_leafPool.m_misses << endl
		<< "Index pool hits: " << t.m_indexPool.m_hits << endl
		<< "Index pool misses: " << t.m_indexPool.m_misses << endl
		<< "Rect pool hits: " << t.m_rectPool.m_hits << endl
		<< "Rect pool misses: " << t.m_rectPool.m_misses << endl
		<< "Point pool hits: " << t.m_pointPool.m_hits << endl
		<< "Point pool misses: " << t.m_pointPool.m_misses << endl;
#endif

	return os;
}
