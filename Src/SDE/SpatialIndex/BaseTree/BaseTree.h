#ifndef _IS_SDE_SPATIALINDEX_BASETREE_H_
#define _IS_SDE_SPATIALINDEX_BASETREE_H_
#include "../Utility/Utility.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Node;
			class Index;
			class Leaf;
			class Overflow;
			class BulkLoader;
		}

		namespace RTree
		{
			class Node;
			class Index;
			class Leaf;
			class BulkLoader;
		}

		namespace NVRTree
		{
			class Node;
			class Leaf;
			class BulkLoader;
		}

		template<class TNode, class TStatistics>
		class BaseTree : public ISpatialIndex
		{
		public:
			typedef Tools::PoolPointer<TNode> NodePtr;

		public:
			BaseTree(IStorageManager& sm, Tools::PropertySet& ps, size_t indexCapacity, size_t leafCapacity, id_type headerID);
			virtual ~BaseTree();

			//
			// ISpatialIndex interface
			//
			virtual void insertData(size_t len, const byte* pData, const IShape& shape, id_type objID, id_type* nodeID);
			virtual bool deleteData(const IShape& shape, id_type id);
			virtual void containsWhatQuery(const IShape& query, IVisitor& v, bool isFine);
			virtual void intersectsWithQuery(const IShape& query, IVisitor& v, bool isFine);
			virtual void pointLocationQuery(const Point& query, IVisitor& v, bool isFine);
			virtual void queryStrategy(IQueryStrategy& qs);
			virtual void addCommand(ICommand* pCommand, CommandType ct);

			virtual void readData(id_type nodeID, id_type objID, IShape** out);
			virtual id_type nextObjID();
			virtual void setDisplayRegion(const Rect& scope);
			virtual const Rect& getDisplayRegion();

		protected:
			virtual void initNew(Tools::PropertySet& ps);
			virtual void initOld(Tools::PropertySet& ps);
			virtual void storeHeader();
			virtual void loadHeader();

			virtual void insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID/*, bool useOverflow = true*/);
			virtual bool deleteData_impl(const Rect& mbr, id_type id);

			virtual id_type writeNode(TNode*);
			virtual NodePtr readNode(id_type id);
			virtual void deleteNode(TNode*);

			virtual void rangeQuery(RangeQueryType type, const IShape& query, IVisitor&, bool) = 0;

			void setFirstDisplayScope();

		private:
			static const int DIMENSION = 2;

		protected:
			IStorageManager* m_pStorageManager;

			id_type m_rootID;
			id_type m_headerID;

			size_t m_indexCapacity;
			size_t m_leafCapacity;

			Rect m_infiniteRect;
			Rect m_firstDisplayScope;

			TStatistics m_stats;

			id_type m_nextObjID;

			Tools::PointerPool<Point> m_pointPool;
			Tools::PointerPool<Rect> m_rectPool;
			Tools::PointerPool<TNode> m_indexPool;
			Tools::PointerPool<TNode> m_leafPool;

			std::vector<Tools::SmartPointer<ICommand> > m_writeNodeCommands;
			std::vector<Tools::SmartPointer<ICommand> > m_readNodeCommands;
			std::vector<Tools::SmartPointer<ICommand> > m_deleteNodeCommands;

#ifdef PTHREADS
			pthread_rwlock_t m_rwLock;
#else
			bool m_rwLock;
#endif

			friend class QuadTree::Node;
			friend class QuadTree::Index;
			friend class QuadTree::Leaf;
			friend class QuadTree::Overflow;
			friend class QuadTree::BulkLoader;

			friend class RTree::Node;
			friend class RTree::Index;
			friend class RTree::Leaf;
			friend class RTree::BulkLoader;

			friend class NVRTree::Node;
			friend class NVRTree::Leaf;
			friend class NVRTree::BulkLoader;

		};	// BaseTree
	}
}

#include "BaseTree.cpp"

#endif // _IS_SDE_SPATIALINDEX_BASETREE_H_