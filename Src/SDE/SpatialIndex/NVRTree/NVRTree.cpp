#include "NVRTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "BulkLoader.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::NVRTree;

ISpatialIndex* IS_SDE::SpatialIndex::NVRTree::returnNVRTree(IStorageManager& sm, Tools::PropertySet& ps)
{
	ISpatialIndex* si = new SpatialIndex::NVRTree::NVRTree(sm, ps);
	return si;
}

ISpatialIndex* IS_SDE::SpatialIndex::NVRTree::createNewNVRTree(
	IStorageManager& sm,
	double fillFactor,
	size_t indexCapacity,
	size_t leafCapacity,
	id_type& indexIdentifier
	)
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

	ISpatialIndex* ret = returnNVRTree(sm, ps);

	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.lVal;

	return ret;
}

ISpatialIndex* IS_SDE::SpatialIndex::NVRTree::createAndBulkLoadNewNVRTree(
	BulkLoadMethod m,
	IDataStream& stream,
	IStorageManager& sm,
	double fillFactor,
	size_t indexCapacity,
	size_t leafCapacity,
	id_type& indexIdentifier)
{
	ISpatialIndex* tree = createNewNVRTree(sm, fillFactor, indexCapacity, leafCapacity, indexIdentifier);

	size_t bindex = static_cast<size_t>(std::floor(static_cast<double>(indexCapacity * fillFactor)));
	size_t bleaf = static_cast<size_t>(std::floor(static_cast<double>(leafCapacity * fillFactor)));

	BulkLoader bl;

	switch (m)
	{
	case BLM_STR:
		//FIXME: find available memory here.
		bl.bulkLoadUsingSTR(static_cast<NVRTree*>(tree), stream, bindex, bleaf, 200000);
		break;
	default:
		throw Tools::IllegalArgumentException("createAndBulkLoadNewRTree: Unknown bulk load method.");
		break;
	}

	return tree;
}

ISpatialIndex* IS_SDE::SpatialIndex::NVRTree::loadNVRTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = static_cast<int32_t>(indexIdentifier);
	ps.setProperty("IndexIdentifier", var);

	return returnNVRTree(sm, ps);
}

IS_SDE::SpatialIndex::NVRTree::NVRTree::NVRTree(IStorageManager& sm, Tools::PropertySet& ps) 
:	BaseTree(sm, ps, 100, 100, StorageManager::NewPage),
	m_fillFactor(0.7)
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

IS_SDE::SpatialIndex::NVRTree::NVRTree::~NVRTree()
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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc)
{
	Tools::NotSupportedException("Should never be called. ");
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	Tools::NotSupportedException("Should never be called. ");
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::selfJoinQuery(const IShape& query, IVisitor& v)
{
	Tools::NotSupportedException("Should never be called. ");
}

bool IS_SDE::SpatialIndex::NVRTree::NVRTree::countIntersectsQuery(const IShape& query, ICountVisitor& v)
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
			IShape* nodeMBR;
			n->getShape(&nodeMBR);
			bool bNodeIn = query.containsShape(*nodeMBR);
			delete nodeMBR;

			if (bNodeIn)
			{
				v.countNode(*n.get());
				if (v.overUpperLimit())
					return true;
			}
			else
			{
				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (query.intersectsShape(*(n->m_ptrMBR[cChild])))
					{
						v.countData(*n.get(), cChild, query);
						if (v.overUpperLimit())
							return true;
					}
				}
			}
		}
		else
		{
			for (size_t cChild = 0; cChild < n->m_children; cChild++)
			{
				if (query.intersectsShape(*(n->m_ptrMBR[cChild]))) 
					st.push(readNode(n->m_pIdentifier[cChild]));
			}
		}
	}

	return false;
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::getIndexProperties(Tools::PropertySet& out) const
{
	Tools::Variant var;

	// fill factor
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_fillFactor;
	out.setProperty("FillFactor", var);

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

bool IS_SDE::SpatialIndex::NVRTree::NVRTree::isIndexValid()
{
	Tools::NotSupportedException("Should never be called. ");
	return true;
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::getStatistics(IStatistics** out) const
{
	*out = new Statistics(m_stats);
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// fill factor
	// it cannot be larger than 50%, since linear and quadratic split algorithms
	// require assigning to both nodes the same number of entries.
	var = ps.getProperty("FillFactor");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_DOUBLE ||
			var.m_val.dblVal < 0.0 ||
			var.m_val.dblVal > 1.0)
			throw Tools::IllegalArgumentException("initNew: Property FillFactor must be Tools::VT_DOUBLE and in (0.0, 1.0) for RSTAR");

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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::storeHeader()
{
	const size_t headerSize =
		sizeof(id_type) +						// m_rootID
		sizeof(id_type) +						// m_nextObjID
		sizeof(double)	+                       // m_fillFactor
		sizeof(size_t)	+                       // m_indexCapacity
		sizeof(size_t)	+                       // m_leafCapacity
		sizeof(Rect)	+						// m_firstDisplayScope
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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::loadHeader()
{
	size_t headerSize;
	byte* header = 0;
	m_pStorageManager->loadByteArray(m_headerID, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_rootID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_nextObjID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
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

id_type IS_SDE::SpatialIndex::NVRTree::NVRTree::writeNode(Node* n)
{
	byte* buffer;
	size_t dataLength;
	n->storeToByteArray(&buffer, dataLength);

	id_type page;
	if (n->m_identifier < 0) page = StorageManager::NewPage;
	else page = n->m_identifier;

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

NodePtr IS_SDE::SpatialIndex::NVRTree::NVRTree::readNode(id_type id)
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

		n->m_pNVRTree = this;
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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::deleteNode(Node* n)
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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v, bool isFine)
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

void IS_SDE::SpatialIndex::NVRTree::NVRTree::findLeafPath(
	id_type nodeID, const Rect& nodeMBR, std::stack<std::pair<id_type, size_t> >* pathBuffer
	)
{
	NodePtr root = readNode(m_rootID);
	if (root->isLeaf())
	{
		assert(root->getIdentifier() == nodeID);
		return;
	}

	bool bFind = root->findLeafPath(nodeID, nodeMBR, pathBuffer);
	assert(bFind);
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::adjustTree(const NodePtr& n, const Rect& originalMBR)
{
	std::stack<std::pair<id_type, size_t> > pathBuffer;
	findLeafPath(n->getIdentifier(), originalMBR, &pathBuffer);

	if (pathBuffer.empty())
	{
		assert(n->getIdentifier() == m_rootID);
	}
	else
	{
		std::pair<id_type, size_t> pos = pathBuffer.top(); pathBuffer.pop();
		NodePtr ptrN = readNode(pos.first);
		Index* p = static_cast<Index*>(ptrN.get());
		p->adjustTree(n.get(), pos.second, &pathBuffer);
	}
}

void IS_SDE::SpatialIndex::NVRTree::NVRTree::condenseTree(const NodePtr& n)
{
	std::stack<std::pair<id_type, size_t> > pathBuffer;
	findLeafPath(n->getIdentifier(), n->getNodeMBR(), &pathBuffer);
	n->condenseTree(&pathBuffer);
}