#include "StaticRTree.h"
#include "../../../NVDataPublish/Features/BulkLoader.h"
#include "../../../NVDataPublish/Features/Data.h"
#include "../../../NVDataPublish/Features/Strategy.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::SpatialIndex::StaticRTree;

ISpatialIndex* SimulateNavigate::SpatialIndex::StaticRTree::returnStaticRTree(IStorageManager& sm, Tools::PropertySet& ps)
{
	ISpatialIndex* si = new SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree(sm, ps);
	return si;
}

ISpatialIndex* SimulateNavigate::SpatialIndex::StaticRTree::createNewStaticRTree(
	IStorageManager& sm,
	id_type& indexIdentifier,
	size_t condenseStrategy
	)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = condenseStrategy;
	ps.setProperty("CondenseStrategy", var);

	ISpatialIndex* ret = returnStaticRTree(sm, ps);

	var = ps.getProperty("IndexIdentifier");
	indexIdentifier = var.m_val.lVal;

	return ret;
}

ISpatialIndex* SimulateNavigate::SpatialIndex::StaticRTree::createAndBulkLoadNewStaticRTree(
	BulkLoadMethod m,
	IDataStream& stream,
	IStorageManager& sm,
	NVDataPublish::Features::CondenseData& cd,
	id_type& indexIdentifier,
	size_t condenseStrategy
	)
{
	ISpatialIndex* tree = createNewStaticRTree(sm, indexIdentifier, condenseStrategy);

	NVDataPublish::Features::LayerBulkLoader lbl;

	switch (m)
	{
	case BLM_STR:
		//FIXME: find available memory here.
		lbl.bulkLoadUsingSTR(static_cast<StaticRTree*>(tree), stream, cd, 200000);
		break;
	default:
		throw Tools::IllegalArgumentException("createAndBulkLoadNewStaticRTree: Unknown bulk load method.");
		break;
	}

	return tree;
}

ISpatialIndex* SimulateNavigate::SpatialIndex::StaticRTree::loadStaticRTree(IStorageManager& sm, id_type indexIdentifier)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = static_cast<int32_t>(indexIdentifier);
	ps.setProperty("IndexIdentifier", var);

	return returnStaticRTree(sm, ps);
}

SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	StaticRTree(IStorageManager& sm, Tools::PropertySet& ps) : 
	m_pStorageManager(&sm), 
	m_rootID(0), 
	m_headerID(StorageManager::NewPage),
	m_maxDisplayCount(200),
	m_rectPool(1000),
	m_nodePool(100),
	m_condenseStrategy(NULL)
{
#ifdef PTHREADS
	pthread_rwlock_init(&m_rwLock, NULL);
#else
	m_rwLock = false;
#endif

	Tools::Variant var = ps.getProperty("IndexIdentifier");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG) throw Tools::IllegalArgumentException(
			"StaticRTree: Property IndexIdentifier must be Tools::VT_LONG"
			);
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

SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	~StaticRTree()
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

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	insertData(size_t len, const byte* pData, const IShape& shape, id_type objID, id_type* nodeID)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

bool SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	deleteData(const IShape& shape, id_type shapeIdentifier)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	containsWhatQuery(const IShape& query, IVisitor& v, bool isFine)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	intersectsWithQuery(const IShape& query, IVisitor& v, bool isFine)
{
	rangeQuery(IntersectionQuery, query, v, isFine);
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	pointLocationQuery(const Point& query, IVisitor& v, bool isFine)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	selfJoinQuery(const IShape& s, IVisitor& v)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	queryStrategy(IQueryStrategy& qs)
{
#ifdef PTHREADS
	Tools::SharedLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("queryStrategy: cannot acquire a shared lock");
#endif

	id_type next = m_rootID;
	bool hasNext = true;

	try
	{
		while (hasNext)
		{
			NodePtr n = readNode(next);
			qs.getNextEntry(*n, next, hasNext);

			if (qs.shouldStoreEntry())
				writeNode(n.get());
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

bool SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	countIntersectsQuery(const IShape& query, ICountVisitor& v)
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

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	getIndexProperties(Tools::PropertySet& out) const
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	addCommand(ICommand* in, CommandType ct)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

bool SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	isIndexValid()
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	getStatistics(IStatistics** out) const
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	readData(id_type nodeID, id_type objID, IShape** out)
{
	throw Tools::NotSupportedException("Should never be called. ");
}

id_type SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	nextObjID()
{
	throw Tools::NotSupportedException("Should never be called. ");
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	setDisplayRegion(const Rect& scope)
{
	m_firstDisplayScope = scope;
}

const Rect& SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	getDisplayRegion()
{
	return m_firstDisplayScope;
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	readDataByAddress(id_type nodeID, size_t offset, IShape** s) const
{
	size_t dataLength;
	byte* buffer;

	try
	{
		m_pStorageManager->loadByteArray(nodeID, dataLength, &buffer);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		//std::cerr << *this << std::endl;
		throw Tools::IllegalStateException("readTopoEdge: failed with Tools::InvalidPageException");
	}

	byte* ptr = buffer + offset;
	ptr += Node::MBR_SIZE + Node::EDGE_ID_SIZE;
	size_t len = 0;
	memcpy(&len, ptr, Node::EACH_CHILD_SIZE_SIZE);
	ptr += Node::EACH_CHILD_SIZE_SIZE;
	
	m_condenseStrategy->loadFromByteArray(s, ptr, len);

	delete[] buffer;
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	initNew(Tools::PropertySet& ps)
{
	Tools::Variant var;

	// node pool capacity
	var = ps.getProperty("NodePoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG)
			throw Tools::IllegalArgumentException("initNew: Property IndexPoolCapacity must be Tools::VT_LONG");

		m_nodePool.setCapacity(var.m_val.lVal);
	}

	// rect pool capacity
	var = ps.getProperty("RectPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG)
			throw Tools::IllegalArgumentException("initNew: Property RectPoolCapacity must be Tools::VT_LONG");

		m_rectPool.setCapacity(var.m_val.lVal);
	}

	var = ps.getProperty("CondenseStrategy");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG)
			throw Tools::IllegalArgumentException("initNew: Property CondenseStrategy must be Tools::VT_LONG");

		NVDataPublish::CondenseStrategyType cs 
			= static_cast<NVDataPublish::CondenseStrategyType>(var.m_val.lVal);
		switch (cs)
		{
		case NVDataPublish::CondenseStrategyType::CS_NO_OFFSET:
			m_condenseStrategy = new NVDataPublish::Features::NoOffsetCondenseStrategy;
			break;
		default:
			throw Tools::IllegalArgumentException("Unknown bulk condense strategy.");
			break;
		}
	}

	m_infiniteRect.makeInfinite();

	Node root(this, -1);
	m_rootID = writeNode(&root);

	storeHeader();
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	initOld(Tools::PropertySet& ps)
{
	loadHeader();

	// only some of the properties may be changed.
	// the rest are just ignored.

	Tools::Variant var;

	// index pool capacity
	var = ps.getProperty("NodePoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG) 
			throw Tools::IllegalArgumentException("initOld: Property IndexPoolCapacity must be Tools::VT_LONG");

		m_nodePool.setCapacity(var.m_val.lVal);
	}

	// rect pool capacity
	var = ps.getProperty("RectPoolCapacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_LONG) 
			throw Tools::IllegalArgumentException("initOld: Property RectPoolCapacity must be Tools::VT_LONG");

		m_rectPool.setCapacity(var.m_val.lVal);
	}

	m_infiniteRect.makeInfinite();
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	storeHeader()
{
	const size_t headerSize =
		sizeof(id_type)	+							// m_rootID
		sizeof(Rect) +								// m_firstDisplayScope
		sizeof(size_t); 							// m_condenseStrategy

	byte* header = new byte[headerSize];
	byte* ptr = header;

	memcpy(ptr, &m_rootID, sizeof(id_type));
	ptr += sizeof(id_type);

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

	size_t type = m_condenseStrategy->getType();
	memcpy(ptr, &type, sizeof(size_t));
	ptr += sizeof(size_t);

	m_pStorageManager->storeByteArray(m_headerID, headerSize, header);

	delete[] header;
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	loadHeader()
{
	size_t headerSize;
	byte* header = 0;
	m_pStorageManager->loadByteArray(m_headerID, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_rootID, ptr, sizeof(id_type));
	ptr += sizeof(id_type);

	size_t rectSize = m_firstDisplayScope.getByteArraySize();
	byte* pRect = new byte[rectSize];
	memcpy(pRect, ptr, rectSize);
	m_firstDisplayScope.loadFromByteArray(pRect);
	delete pRect;
	ptr += rectSize;

	size_t type;
	memcpy(&type, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	NVDataPublish::CondenseStrategyType cs 
		= static_cast<NVDataPublish::CondenseStrategyType>(type);
	switch (cs)
	{
	case NVDataPublish::CondenseStrategyType::CS_NO_OFFSET:
		m_condenseStrategy = new NVDataPublish::Features::NoOffsetCondenseStrategy;
		break;
	default:
		throw Tools::IllegalArgumentException("Unknown bulk condense strategy.");
		break;
	}

	delete[] header;
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v, bool isFine)
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
				bool bNodeIn = dynamic_cast<Rect*>(const_cast<IShape*>(&query)) 
					&& query.containsShape(*nodeMBR);
				delete nodeMBR;

				if (bNodeIn)
				{
					v.visitData(*n);

					if (v.shouldCheckEachData())
					{
						for (size_t cChild = 0; cChild < n->m_children; cChild++)
						{
							NVDataPublish::Features::Data data = NVDataPublish::Features::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], m_condenseStrategy);
							v.visitData(data, cChild);
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
									NVDataPublish::Features::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], m_condenseStrategy).getShape(&pS);
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
									NVDataPublish::Features::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], m_condenseStrategy).getShape(&pS);
									b = query.intersectsShape(*pS);
									delete pS;
								}
							}
						}

						if (b)
						{
							NVDataPublish::Features::Data data = NVDataPublish::Features::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], m_condenseStrategy);
							v.visitData(data, cChild);
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

id_type SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	writeNode(Node* n)
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
		n->m_identifier = page;

	return page;
}

NodePtr SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	readNode(id_type id)
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
		NodePtr n = m_nodePool.acquire();

		if (n.get() == 0)
			n = NodePtr(new Node(this, -1), &m_nodePool);

		n->m_tree = this;
		n->m_identifier = id;
		n->m_bLeaf = false;
		n->loadFromByteArray(buffer);

		delete[] buffer;
		return n;
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	deleteNode(Node* n)
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
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	setFirstDisplayScope()
{
	if (m_firstDisplayScope.isEmpty())
		m_firstDisplayScope = readNode(m_rootID)->m_nodeMBR;

	while (true)
	{
		CountRoadVisitor vis(m_maxDisplayCount);
		if (countIntersectsQuery(m_firstDisplayScope, vis))
			m_firstDisplayScope.changeSize(0.5);
		else
			break;
	}
}

//
// class StaticRTree::CountRoadVisitor
//

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	CountRoadVisitor::countNode(const INode& node)
{
	m_count += node.getChildrenCount();	
}

void SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	CountRoadVisitor::countData(const INode& node, size_t iData, const IShape& query)
{
	++m_count;
}

bool SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree::
	CountRoadVisitor::overUpperLimit() const
{
	return m_count > m_upperLimit;
}