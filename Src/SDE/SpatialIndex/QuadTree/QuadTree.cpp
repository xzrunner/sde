#include "QuadTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "BulkLoader.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::QuadTree;

ISpatialIndex* IS_SDE::SpatialIndex::QuadTree::returnQuadTree(IStorageManager& sm, Tools::PropertySet& ps)
{
	ISpatialIndex* si = new SpatialIndex::QuadTree::QuadTree(sm, ps);
	return si;
}

ISpatialIndex* IS_SDE::SpatialIndex::QuadTree::createNewQuadTree(
	IStorageManager& sm,
	double fillFactor,
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
	var.m_val.ulVal = leafCapacity;
	ps.setProperty("LeafCapacity", var);

	ISpatialIndex* ret = returnQuadTree(sm, ps);

	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.llVal;

	return ret;
}

ISpatialIndex* IS_SDE::SpatialIndex::QuadTree::createAndBulkLoadNewQuadTree(
	BulkLoadMethod m,
	IDataStream& stream,
	IStorageManager& sm,
	double fillFactor,
	size_t leafCapacity,
	id_type& indexIdentifier
	)
{
	ISpatialIndex* tree = createNewQuadTree(sm, fillFactor, leafCapacity, indexIdentifier);

	size_t bleaf = static_cast<size_t>(std::floor(static_cast<double>(leafCapacity * fillFactor)));

	BulkLoader bl;

	switch (m)
	{
	case BLM_TYPE0:
		//FIXME: find available memory here.
		bl.bulkLoadUsingType0(static_cast<QuadTree*>(tree), stream, bleaf, 200000);
		break;
	default:
		throw Tools::IllegalArgumentException("createAndBulkLoadNewQuadTree: Unknown bulk load method.");
		break;
	}

	return tree;
}

ISpatialIndex* IS_SDE::SpatialIndex::QuadTree::loadQuadTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = indexIdentifier;
	ps.setProperty("IndexIdentifier", var);

	return returnQuadTree(sm, ps);
}

IS_SDE::SpatialIndex::QuadTree::QuadTree::QuadTree(IStorageManager& sm, Tools::PropertySet& ps) :
	BaseTree(sm, ps, 100, 100, MAX_ID),
	m_overflowID(MAX_ID - 1),
	m_overflowCapacity(2)
{
#ifdef PTHREADS
	pthread_rwlock_init(&m_rwLock, NULL);
#else
	m_rwLock = false;
#endif

	Tools::Variant var = ps.getProperty("IndexIdentifier");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONGLONG) 
			throw Tools::IllegalArgumentException("QuadTree: Property IndexIdentifier must be Tools::VT_LONGLONG");
		m_headerID = var.m_val.llVal;
		initOld(ps);
	}
	else
	{
		initNew(ps);
		var.m_varType = Tools::VT_LONGLONG;
		var.m_val.llVal = m_headerID;
		ps.setProperty("IndexIdentifier", var);
	}
}

IS_SDE::SpatialIndex::QuadTree::QuadTree::~QuadTree()
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

void IS_SDE::SpatialIndex::QuadTree::QuadTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator&)
{
	throw Tools::NotSupportedException("Not Support! ");
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	throw Tools::NotSupportedException("Not Support! ");
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::selfJoinQuery(const IShape& s, IVisitor& v)
{
	throw Tools::NotSupportedException("Not Support! ");
}

bool IS_SDE::SpatialIndex::QuadTree::QuadTree::countIntersectsQuery(const IShape& query, ICountVisitor& v)
{
	throw Tools::NotSupportedException("Not Support! ");
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::getIndexProperties(Tools::PropertySet& out) const
{
	throw Tools::NotSupportedException("Not Support! ");
}

bool IS_SDE::SpatialIndex::QuadTree::QuadTree::isIndexValid()
{
	throw Tools::NotSupportedException("Not Support! ");
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::getStatistics(IStatistics** out) const
{
	throw Tools::NotSupportedException("Not Support! ");
}

//void IS_SDE::SpatialIndex::QuadTree::QuadTree::checkStoreState(ICheckStoreStrategy& css)
//{
//	size_t length, pageSize;
//	m_pStorageManager->getNodePageInfo(HEADER_PHYSICAL_ID, length, pageSize);
//	// todo
//	CheckStoreStrategy* cssCast = dynamic_cast<CheckStoreStrategy*>(&css);
//	++cssCast->m_leafNum;
//	cssCast->m_leafPageNum += pageSize;
//	cssCast->m_leafSize += length;
//
//	id_type nextLogicalID = m_rootID;
//	bool hasNext = true;
//	while (hasNext)
//	{
//		NodePtr n = readNode(nextLogicalID);
//		id_type nextPhysicalID = m_IDManager.transIDLogicalToPhysical(nextLogicalID);
//		css.getNextEntry(m_pStorageManager, *n, nextPhysicalID, hasNext);
//		nextLogicalID = nextPhysicalID;
//	}
//}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// index capacity
	m_indexCapacity = INDEX_CHILDREN_SIZE;

	// leaf capacity
	var = ps.getProperty("LeafCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 4)
			throw Tools::IllegalArgumentException("initNew: Property LeafCapacity must be Tools::VT_ULONG and >= 4");

		m_leafCapacity = var.m_val.ulVal;
	}

	// overflow capacity
	var = ps.getProperty("OverflowCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 10)
			throw Tools::IllegalArgumentException("initNew: Property OverflowCapacity must be Tools::VT_ULONG and >= 10");

		m_overflowCapacity = var.m_val.ulVal;
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

	// make sure the phycisal ID of header is 0
	storeHeader();

	Overflow overflow(this, m_overflowID);
	overflow.m_level = 0;
	overflow.m_identifier = m_overflowID;
	writeNode(&overflow);

	Leaf root(this, m_rootID);
	overflow.m_level = 0;
	overflow.m_identifier = m_rootID;
	writeNode(&root);
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

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

void IS_SDE::SpatialIndex::QuadTree::QuadTree::storeHeader()
{
	const size_t sizeofIDManager = m_IDManager.getByteArraySize();
	const size_t headerSize =
		sizeof(id_type)	+						// m_rootID
		sizeof(id_type)	+						// m_nextObjID
		sizeof(size_t)	+						// m_leafCapacity
		sizeof(size_t)	+						// m_overflowCapacity
		m_firstDisplayScope.getByteArraySize() +		// m_firstDisplayScope
		sizeofIDManager + sizeof(size_t) +		// m_IDManager	
		sizeof(size_t)	+						// m_stats.m_nodes
		sizeof(size_t)	+						// m_stats.m_data
		sizeof(size_t)	+						// m_stats.m_treeHeight
		m_stats.m_treeHeight * sizeof(size_t);	// m_stats.m_nodesInLevel

	byte* header = new byte[headerSize];
	byte* ptr = header;

	memcpy(ptr, &m_rootID, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_nextObjID, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_leafCapacity, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_overflowCapacity, sizeof(size_t));
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
	
	memcpy(ptr, &sizeofIDManager, sizeof(size_t));
	ptr += sizeof(size_t);
	byte* pIDMamager; size_t len;
	m_IDManager.storeToByteArray(&pIDMamager, len);
	memcpy(ptr, pIDMamager, sizeofIDManager);
	ptr += sizeofIDManager;
	delete[] pIDMamager;

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

	id_type page = m_IDManager.getPhysicalIDToWrite(m_headerID);
	m_pStorageManager->storeByteArray(page, headerSize, header);
	m_IDManager.updateTransTable(m_headerID, page);

	delete[] header;
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::loadHeader()
{
	size_t headerSize;
	byte* header = 0;

	// physicalID always is 0
	// because it is always the first node to store
	const id_type headerPhysicalID = 0;
	m_pStorageManager->loadByteArray(headerPhysicalID, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_rootID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_nextObjID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(&m_leafCapacity, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_overflowCapacity, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	size_t rectSize = m_firstDisplayScope.getByteArraySize();
	byte* pRect = new byte[rectSize];
	memcpy(pRect, ptr, rectSize);
	m_firstDisplayScope.loadFromByteArray(pRect);
	delete pRect;
	ptr += rectSize;

	size_t idManagerSize;
	memcpy(&idManagerSize, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	byte* pIDManager = new byte[idManagerSize];
	memcpy(pIDManager, ptr, idManagerSize);
	m_IDManager.loadFromByteArray(pIDManager);
	delete pIDManager;
	ptr += idManagerSize;

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

void IS_SDE::SpatialIndex::QuadTree::QuadTree::insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID)
{
	NodePtr root = readNode(m_rootID);
	if (!root->m_nodeMBR.containsRect(mbr))
	{
		NodePtr overflow = readNode(m_overflowID);
		assert(overflow->isLeaf());
		Overflow* p = static_cast<Overflow*>(overflow.get());
		*nodeID = p->getIdentifier();
		p->insertData(dataLength, pData, mbr, objID);
	}
	else
	{
		std::vector<id_type> destLeaves;
		root->chooseSubtree(mbr, destLeaves);

		if (destLeaves.empty())
			throw Tools::IllegalStateException("QuadTree::insertData_impl: Didn't find a leaf to insert. ");

		for (size_t i = 0; i < destLeaves.size(); ++i)
		{
			NodePtr node = readNode(destLeaves.at(i));
			assert(node->isLeaf());
			if (i == 0)
				*nodeID = node->getIdentifier();
			static_cast<Leaf*>(node.get())->insertData(dataLength, pData, mbr, objID);
		}

		if (destLeaves.size() > 1)
			++m_stats.m_duplicateObj;
		m_stats.m_duplicateTotal += destLeaves.size() - 1;
		++m_stats.m_data;
	}
}

bool IS_SDE::SpatialIndex::QuadTree::QuadTree::deleteData_impl(const Rect& mbr, id_type id)
{
	bool ret = false;

	std::vector<id_type> destLeaves;

	NodePtr root = readNode(m_rootID);
	if (root->m_nodeMBR.intersectsRect(mbr))
	{
		root->chooseSubtree(mbr, destLeaves);
	}
	else
	{
		NodePtr overflow = readNode(m_overflowID);
		if (overflow->m_nodeMBR.intersectsRect(mbr))
			destLeaves.push_back(m_overflowID);
	}

	if (destLeaves.empty())
		throw Tools::IllegalStateException("QuadTree::deleteData_impl: Didn't find a leaf. ");

	for(size_t i = 0; i < destLeaves.size(); ++i)
	{
		NodePtr node = readNode(destLeaves.at(i));
		static_cast<Leaf*>(node.get())->deleteData(id);
		ret = true;
	}

	return ret;
}

id_type IS_SDE::SpatialIndex::QuadTree::QuadTree::writeNode(Node* n)
{
	byte* buffer;
	size_t dataLength;
	n->storeToByteArray(&buffer, dataLength);

	id_type page = m_IDManager.getPhysicalIDToWrite(n->m_identifier);
	bool isNewPage = page == StorageManager::NewPage;

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
	}

	// when create overflow node, it is a new node whild m_identifier > 0.
	if (isNewPage)
	{
		m_stats.m_nodes++;

		if (m_stats.m_nodesInLevel.size() == n->m_level)
		{
			m_stats.m_nodesInLevel.push_back(0);
			++m_stats.m_treeHeight;
		}
		m_stats.m_nodesInLevel[n->m_level] = m_stats.m_nodesInLevel[n->m_level] + 1;

		m_IDManager.updateTransTable(n->m_identifier, page);
	}

	m_stats.m_writes++;

	for (size_t cIndex = 0; cIndex < m_writeNodeCommands.size(); cIndex++)
	{
		m_writeNodeCommands[cIndex]->execute(*n);
	}

	return page;
}

NodePtr IS_SDE::SpatialIndex::QuadTree::QuadTree::readNode(id_type id)
{
	size_t dataLength;
	byte* buffer;

	id_type physicalID = m_IDManager.transIDLogicalToPhysical(id);

	try
	{
		m_pStorageManager->loadByteArray(physicalID, dataLength, &buffer);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("readNode: failed with Tools::InvalidPageException");
	}

	try
	{
		NodeType nodeType;
		memcpy(&nodeType, buffer, sizeof(size_t));

		NodePtr n;

		if (nodeType == NT_Index) 
			n = m_indexPool.acquire();
		else if (nodeType == NT_Leaf) 
			n = m_leafPool.acquire();
		else if (nodeType == NT_Overflow)
			;
		else 
			throw Tools::IllegalStateException("readNode: failed reading the correct node type information");

		if (n.get() == 0)
		{
			if (nodeType == NT_Index) 
				n = NodePtr(new Index(this, -1, 0), &m_indexPool);
			else if (nodeType == NT_Leaf) 
				n = NodePtr(new Leaf(this, -1), &m_leafPool);
			else if (nodeType == NT_Overflow)
				n = NodePtr(new Overflow(this, -1), 0);
			else
				throw Tools::IllegalStateException("readNode: failed reading the correct node type information");
		}

		n->m_pQTree = this;
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

void IS_SDE::SpatialIndex::QuadTree::QuadTree::deleteNode(Node* n)
{
	id_type physicalID = m_IDManager.transIDLogicalToPhysical(n->m_identifier, true);

	try
	{
		m_pStorageManager->deleteByteArray(physicalID);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("deleteNode: failed with Tools::InvalidPageException");
	}

	--m_stats.m_nodes;
	--m_stats.m_nodesInLevel.at(n->m_level);

	if (m_stats.m_nodesInLevel.at(n->m_level) == 0)
	{
		assert(n->m_level == m_stats.m_nodesInLevel.size() - 1 
			&& n->m_level + 1 == m_stats.m_treeHeight);
		--m_stats.m_treeHeight;
		m_stats.m_nodesInLevel.pop_back();
	}

	for (size_t cIndex = 0; cIndex < m_deleteNodeCommands.size(); cIndex++)
	{
		m_deleteNodeCommands[cIndex]->execute(*n);
	}
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v, bool isFine)
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

		// the difference compared with RTree
		NodePtr overflow = readNode(m_overflowID);
		if (query.intersectsShape(overflow->m_nodeMBR)) 
			st.push(overflow);
		//

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
			else
			{
				v.visitNode(*n);

				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (query.intersectsShape(*(n->m_ptrMBR[cChild]))) 
						st.push(readNode(n->m_pIdentifier[cChild]));
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

void IS_SDE::SpatialIndex::QuadTree::QuadTree::enlargeToContainOverflowNode(const Rect& overflowScope)
{
	NodePtr rootNode = readNode(m_rootID);
	double enlargeMultiple = 0.0;
	for (size_t i = 0; i < 2; ++i)
	{
		double halfLengthRoot = 
			0.5 * (rootNode->m_nodeMBR.m_pHigh[i] - rootNode->m_nodeMBR.m_pLow[i]);
		double centerCoordsRoot = rootNode->m_nodeMBR.m_pLow[i] + halfLengthRoot;

		double times = std::max(fabs(overflowScope.m_pLow[i] - centerCoordsRoot), 
			fabs(overflowScope.m_pHigh[i] - centerCoordsRoot)) / halfLengthRoot;
		if (times > enlargeMultiple)
			enlargeMultiple = times;
	}

	assert(enlargeMultiple > 1.0);

	// enlarge one times, the edge length will equal 2 times of former's
	size_t enlargeTimes = static_cast<size_t>(ceil( log(enlargeMultiple) / log(2.0) ));
	enlargeMultiple = pow(2.0, static_cast<int>(enlargeTimes));

	rootNode->m_nodeMBR.changeSize(enlargeMultiple);
	writeNode(rootNode.get());

	if (rootNode->isIndex())
	{
		if (enlargeTimes > 0)
		{
			SpatialIndex::QuadTree::ChangeAllNodeID::changeAllNodeID(enlargeTimes, this);

			rootNode->clearAllEntry();
			for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
			{
				id_type childID = i + 1;
				NodePtr child = m_indexPool.acquire();
				if (child.get() == NULL)
					child = NodePtr(new Index(this, childID, 1));
				child->m_level = 1;
				child->m_identifier = childID;
				rootNode->setChildRegion((IndexChildPos)i, &child->m_nodeMBR);

				size_t len;
				byte* data;
				child->storeToByteArray(&data, len);
				writeNode(child.get());
				rootNode->insertEntry(len, data, child->m_nodeMBR, childID);
			}
			writeNode(rootNode.get());

			for (size_t iPosition = 0; iPosition < 4; ++iPosition)
			{
				id_type parentID = iPosition + 1;
				size_t positionToExtendNextLayer = 4 - iPosition;
				size_t level = 2;

				for (size_t iLayer = 0; iLayer < enlargeTimes; ++iLayer, ++level)
				{
					NodePtr parentNode = readNode(parentID);
					for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
					{
						id_type childID = parentID * 4 + i + 1;

						NodePtr child;
						if (i + 1 != positionToExtendNextLayer)
						{
							child = m_leafPool.acquire();
							if (child.get() == NULL)
								child  = NodePtr(new Leaf(this, childID));
						}
						else
						{
							if (iLayer == enlargeTimes - 1)
							{
								child = readNode(childID);
							}
							else
							{
								child = m_indexPool.acquire();
								if (child.get() == NULL)
									child = NodePtr(new Index(this, childID, level));
							}
						}
						child->m_level = level;
						child->m_identifier = childID;
						parentNode->setChildRegion((IndexChildPos)i, &child->m_nodeMBR);

						size_t len;
						byte* data;
						child->storeToByteArray(&data, len);
						writeNode(child.get());
						parentNode->insertEntry(len, data, child->m_nodeMBR, childID);
					}
					parentID = parentID * 4 + positionToExtendNextLayer;
					writeNode(parentNode.get());
				}
			}
		}
	}
}

//
//	class IDManager
//

size_t IS_SDE::SpatialIndex::QuadTree::QuadTree::
	   IDManager::getByteArraySize() const
{
	return sizeof(size_t) + m_mapIDLogicalToPhysical.size() * 2 * sizeof(id_type);
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::
	 IDManager::loadFromByteArray(const byte* data)
{
	size_t size;
	memcpy(&size, data, sizeof(size_t));
	data += sizeof(size_t);
	for (size_t i = 0; i < size; ++i)
	{
		id_type logicalID, physicalID;
		memcpy(&logicalID, data, sizeof(id_type));
		data += sizeof(id_type);
		memcpy(&physicalID, data, sizeof(id_type));
		data += sizeof(id_type);
		m_mapIDLogicalToPhysical.insert(std::make_pair(logicalID, physicalID));
	}
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::
	 IDManager::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	size_t size = m_mapIDLogicalToPhysical.size();
	memcpy(ptr, &size, sizeof(size_t));
	ptr += sizeof(size_t);
	std::map<id_type, id_type>::const_iterator itrMap = m_mapIDLogicalToPhysical.begin();
	for ( ; itrMap != m_mapIDLogicalToPhysical.end(); ++itrMap)
	{
		memcpy(ptr, &itrMap->first, sizeof(id_type));
		ptr += sizeof(id_type);
		memcpy(ptr, &itrMap->second, sizeof(id_type));
		ptr += sizeof(id_type);
	}
}

id_type IS_SDE::SpatialIndex::QuadTree::QuadTree::
		IDManager::getPhysicalIDToWrite(id_type logicalID) const
{
	id_type physicalID;
	if (logicalID < 0)
		physicalID = StorageManager::NewPage;
	else
	{
		std::map<id_type, id_type>::const_iterator itr 
			= m_mapIDLogicalToPhysical.find(logicalID);
		if (itr == m_mapIDLogicalToPhysical.end())
			physicalID = StorageManager::NewPage;
		else
			physicalID = itr->second;
	}

	return physicalID;
}

void IS_SDE::SpatialIndex::QuadTree::QuadTree::
	 IDManager::updateTransTable(id_type logicalID, id_type physicalID)
{
	std::map<id_type, id_type>::iterator itr 
		= m_mapIDLogicalToPhysical.find(logicalID);
	if (itr == m_mapIDLogicalToPhysical.end())
		m_mapIDLogicalToPhysical.insert(std::make_pair(logicalID, physicalID));
	else
	{
		if (itr->second != physicalID)
		{
			throw Tools::IllegalStateException(
				"IDManager::updateTransTable: this logical id already has a different physical id. "
				);
		}
	}
}

id_type IS_SDE::SpatialIndex::QuadTree::QuadTree::
		IDManager::transIDLogicalToPhysical(id_type logicalID, bool isDelete/* = false*/)
{
	id_type physicalID;
	std::map<id_type, id_type>::iterator itr 
		= m_mapIDLogicalToPhysical.find(logicalID);
	if (itr != m_mapIDLogicalToPhysical.end())
	{
		physicalID = itr->second;
		if (isDelete)
			m_mapIDLogicalToPhysical.erase(itr);
	}
	else
	{
		throw Tools::IllegalStateException(
			"IDManager::transIDLogicalToPhysical: this logical id not in the table. "
			);
	}

	return physicalID;
}

//
//	class ChangeAllNodeID
//

void IS_SDE::SpatialIndex::QuadTree::
	 ChangeAllNodeID::changeAllNodeID(size_t enlargeTimes, QuadTree* pQuadTree)
{
	std::vector<std::pair<id_type, id_type> > pairOldIDWithNewID;
	pairOldIDWithNewID.reserve(pQuadTree->m_IDManager.m_mapIDLogicalToPhysical.size());

	QuadTree::IDManager tmpIDMgr;
	std::map<id_type, id_type>::iterator itr = pQuadTree->m_IDManager.m_mapIDLogicalToPhysical.begin(),
		itrEnd = pQuadTree->m_IDManager.m_mapIDLogicalToPhysical.end();
	for ( ; itr != itrEnd; ++itr)
	{
		if (itr->first < MAX_ID - 1)
		{
			id_type oldNodeID = itr->first, newNodeID = getNewNodeID(oldNodeID, enlargeTimes);
			pairOldIDWithNewID.push_back(std::make_pair(oldNodeID, newNodeID));

			NodePtr node = pQuadTree->readNode(oldNodeID);
			if (node->isIndex())
			{
				for (size_t i = 0; i < node->m_children; ++i)
				{
					node->m_pIdentifier[i] = getNewNodeID(node->m_pIdentifier[i], enlargeTimes);
				}
			}
			pQuadTree->writeNode(node.get());

			tmpIDMgr.updateTransTable(newNodeID, itr->second);
		}
		else
		{
			tmpIDMgr.updateTransTable(itr->first, itr->second);
		}
	}

	pQuadTree->m_IDManager = tmpIDMgr;
	for (size_t i = pairOldIDWithNewID.size() - 1; i > 0; --i)
	{
		NodePtr node = pQuadTree->readNode(pairOldIDWithNewID.at(i).second);
		assert(node->getIdentifier() == pairOldIDWithNewID.at(i).second);
//		node->m_identifier = pairOldIDWithNewID.at(i).second;
		node->m_level += enlargeTimes;
		pQuadTree->writeNode(node.get());
	}
}

// each nodeID can trans to a serial number, which indicate the position
// old nodeID to new nodeID
// 1, *, *... => 1, 4, *, *...
// 2, *, *... => 2, 3, *, *...
// 3, *, *... => 3, 2, *, *...
// 4, *, *... => 4, 1, *, *...
id_type IS_SDE::SpatialIndex::QuadTree::
		ChangeAllNodeID::getNodeIDAfterRootEnlargeOneTime(id_type oldNodeID)
{
	id_type ret = 0;

	std::stack<byte> positions;

	while (oldNodeID != 0)
	{
		size_t parentID = static_cast<size_t>(
			static_cast<double>(oldNodeID - 1) / static_cast<double>(INDEX_CHILDREN_SIZE)
			);
		positions.push(oldNodeID - parentID * INDEX_CHILDREN_SIZE);
		oldNodeID = parentID;
	}

	bool isFirst = true;
	while (!positions.empty())
	{
		byte position = positions.top(); positions.pop();

		if (isFirst)
		{
			switch (position)
			{
			case 1:
				ret = ret * 4 + position;
				ret = ret * 4 + 4;
				break;
			case 2:
				ret = ret * 4 + position;
				ret = ret * 4 + 3;
				break;
			case 3:
				ret = ret * 4 + position;
				ret = ret * 4 + 2;
				break;
			case 4:
				ret = ret * 4 + position;
				ret = ret * 4 + 1;
				break;
			}

			isFirst = false;
		}
		else
		{
			ret = ret * 4 + position;
		}
	}

	return ret;
}

id_type IS_SDE::SpatialIndex::QuadTree::
		ChangeAllNodeID::getNewNodeID(id_type oldNodeID, size_t times)
{
	id_type ret = oldNodeID;
	for (size_t i = 0; i < times; ++i)
	{
		ret = getNodeIDAfterRootEnlargeOneTime(ret);
	}

	return ret;
}