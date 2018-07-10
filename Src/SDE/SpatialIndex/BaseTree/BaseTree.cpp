#ifndef _IS_SDE_SPATIALINDEX_BASETREE_CPP_
#define _IS_SDE_SPATIALINDEX_BASETREE_CPP_
#include "BaseTree.h"
#include "../Utility/Data.h"
#include "../QuadTree/Node.h"
#include "../RTree/Node.h"

// todo: for setFirstDisplayScope()
#include "../Utility/Visitor.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex;

template<class TNode, class TStatistics>
BaseTree<TNode, TStatistics>::BaseTree(
	IStorageManager& sm, 
	Tools::PropertySet& ps,
	size_t indexCapacity, 
	size_t leafCapacity,
	id_type headerID
	) : 
	m_pStorageManager(&sm),
	m_rootID(0),
	m_headerID(headerID),
	m_indexCapacity(indexCapacity),
	m_leafCapacity(leafCapacity),
	m_pointPool(500),
	m_rectPool(1000),
	m_indexPool(100),
	m_leafPool(100),
	m_nextObjID(0)
{
}

template<class TNode, class TStatistics>
BaseTree<TNode, TStatistics>::~BaseTree()
{
}

//
// ISpatialIndex interface
//

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::insertData(size_t len, const byte* pData, const IShape& shape, id_type objID, id_type* nodeID)
{
#ifdef PTHREADS
	Tools::ExclusiveLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("insertData: cannot acquire an exclusive lock");
#endif

	try
	{
		// convert the shape into a Rect (R-Trees index rects only; i.e., approximations of the shapes).
		RectPtr mbr = m_rectPool.acquire();
		shape.getMBR(*mbr);

		byte* buffer = 0;

		if (len > 0)
		{
			buffer = new byte[len];
			memcpy(buffer, pData, len);
		}

		insertData_impl(len, buffer, *mbr, objID, nodeID);
		// the buffer is stored in the tree. Do not delete here.

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

template<class TNode, class TStatistics>
bool BaseTree<TNode, TStatistics>::deleteData(const IShape& shape, id_type id)
{
#ifdef PTHREADS
	Tools::ExclusiveLock lock(&m_rwLock);
#else
	if (m_rwLock == false) m_rwLock = true;
	else throw Tools::ResourceLockedException("deleteData: cannot acquire an exclusive lock");
#endif

	try
	{
		RectPtr mbr = m_rectPool.acquire();
		shape.getMBR(*mbr);
		bool ret = deleteData_impl(*mbr, id);

#ifndef PTHREADS
		m_rwLock = false;
#endif

		return ret;
	}
	catch (...)
	{
#ifndef PTHREADS
		m_rwLock = false;
#endif
		throw;
	}
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::containsWhatQuery(const IShape& query, IVisitor& v, bool isFine)
{
	rangeQuery(ContainmentQuery, query, v, isFine);
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::intersectsWithQuery(const IShape& query, IVisitor& v, bool isFine)
{
	rangeQuery(IntersectionQuery, query, v, isFine);
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::pointLocationQuery(const Point& query, IVisitor& v, bool isFine)
{
	// todo !
	Rect r(query, 0.00001);
//	Rect r(query, query);
	rangeQuery(IntersectionQuery, r, v, isFine);
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::queryStrategy(IQueryStrategy& qs)
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

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::addCommand(ICommand* pCommand, CommandType ct)
{
	switch (ct)
	{
	case CT_NODEREAD:
		m_readNodeCommands.push_back(Tools::SmartPointer<ICommand>(pCommand));
		break;
	case CT_NODEWRITE:
		m_writeNodeCommands.push_back(Tools::SmartPointer<ICommand>(pCommand));
		break;
	case CT_NODEDELETE:
		m_deleteNodeCommands.push_back(Tools::SmartPointer<ICommand>(pCommand));
		break;
	}
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::readData(id_type nodeID, id_type objID, IShape** out)
{
	NodePtr n = readNode(nodeID);
	n->getChildShape(objID, out);
}

template<class TNode, class TStatistics>
id_type BaseTree<TNode, TStatistics>::nextObjID()
{
	return m_nextObjID++;
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::setDisplayRegion(const Rect& scope)
{
	m_firstDisplayScope = scope;
}

template<class TNode, class TStatistics>
const Rect& BaseTree<TNode, TStatistics>::getDisplayRegion()
{
	return m_firstDisplayScope;
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::initNew(Tools::PropertySet& ps)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::initOld(Tools::PropertySet& ps)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::storeHeader()
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::loadHeader()
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID/*, bool useOverflow = true*/)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
bool BaseTree<TNode, TStatistics>::deleteData_impl(const Rect& mbr, id_type id)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
id_type BaseTree<TNode, TStatistics>::writeNode(TNode*)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
Tools::PoolPointer<TNode> BaseTree<TNode, TStatistics>::readNode(id_type id)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::deleteNode(TNode*)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

template<class TNode, class TStatistics>
void BaseTree<TNode, TStatistics>::setFirstDisplayScope()
{
	if (m_firstDisplayScope.isEmpty())
		m_firstDisplayScope = readNode(m_rootID)->m_nodeMBR;

	bool bFinish = false;
	while (!bFinish)
	{
		std::vector<IShape*> shapes;
		FetchDataVisitor vis(shapes);
		intersectsWithQuery(m_firstDisplayScope, vis, false);

		if (shapes.size() < MAX_DISPLAY_NUM)
		{
			if (shapes.size() == 0)
				m_firstDisplayScope.changeSize(2.0);
			bFinish = true;
		}
		else
			m_firstDisplayScope.changeSize(0.5);

		for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
	}
}

#endif // _IS_SDE_SPATIALINDEX_BASETREE_CPP_