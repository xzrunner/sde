#ifndef _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_STATIC_RTREE_H_
#define _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_STATIC_RTREE_H_
#include "../../../Tools/SpatialDataBase.h"
#include "../../../BasicType/Rect.h"
#include "Utility.h"
#include "PointerPoolNode.h"
#include "../../../NVDataPublish/Base.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class Publish;
			class LayerBulkLoader;
		}
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class StaticRTree : public ISpatialIndex
				{
				public:
					StaticRTree(IStorageManager&, Tools::PropertySet&);
					// String                   Value		Description
					// ----------------------------------------------
					// IndexIdentifier			VT_LONG		If specified an existing index will be openened from the supplied
					//										storage	manager with the given index id. Behaviour is unspecified
					//										if the index id or the storage manager are incorrect.
					// DataType					VT_LONG		One of Point, Line or Polygon
					// NodePoolCapacity			VT_LONG		Default is 100
					// RectPoolCapacity			VT_LONG		Default is 1000
					// PointPoolCapacity        VT_LONG		Default is 500
					// CondenseStrategy			VT_LONG		The method to condense geographic data.

					virtual ~StaticRTree();

				public:
					//
					// ISpatialIndex interface
					//

					virtual void insertData(size_t len, const byte* pData, const IShape& shape, id_type objID, id_type* nodeID);
					virtual bool deleteData(const IShape& shape, id_type shapeIdentifier);
					virtual void containsWhatQuery(const IShape& query, IVisitor& v, bool isFine);
					virtual void intersectsWithQuery(const IShape& query, IVisitor& v, bool isFine);
					virtual void pointLocationQuery(const Point& query, IVisitor& v, bool isFine);
					virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator& nnc);
					virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v);
					virtual void selfJoinQuery(const IShape& s, IVisitor& v);
					virtual void queryStrategy(IQueryStrategy& qs);
					virtual bool countIntersectsQuery(const IShape& query, ICountVisitor& v);
					virtual void getIndexProperties(Tools::PropertySet& out) const;
					virtual void addCommand(ICommand* in, CommandType ct);
					virtual bool isIndexValid();
					virtual void getStatistics(IStatistics** out) const;

					virtual void readData(id_type nodeID, id_type objID, IShape** out);
					virtual id_type nextObjID();
					virtual void setDisplayRegion(const Rect& scope);
					virtual const Rect& getDisplayRegion();

					void readDataByAddress(id_type nodeID, size_t offset, IShape** out) const;

					const NVDataPublish::ICondenseStrategy* getCondenseStrategy() { return m_condenseStrategy; }

				private:
					void initNew(Tools::PropertySet&);
					void initOld(Tools::PropertySet& ps);
					void storeHeader();
					void loadHeader();

					void rangeQuery(RangeQueryType type, const IShape& query, IVisitor&, bool);

					id_type writeNode(Node*);
					NodePtr readNode(id_type id);
					void deleteNode(Node*);

					void setFirstDisplayScope();

				private:
					class CountRoadVisitor : public ICountVisitor
					{
					public:
						CountRoadVisitor(size_t limit) : m_upperLimit(limit), m_count(0) {}

						virtual void countNode(const INode& in);
						virtual void countData(const INode& in, size_t iData, const IShape& query);
						virtual bool overUpperLimit() const;

					private:
						size_t m_upperLimit;
						size_t m_count;

					}; // CountRoadVisitor

				private:
					IStorageManager* m_pStorageManager;

					id_type m_rootID;
					id_type m_headerID;

					Rect m_infiniteRect;
					Rect m_firstDisplayScope;

					size_t m_maxDisplayCount;

					NVDataPublish::ICondenseStrategy* m_condenseStrategy;

					Tools::PointerPool<Rect> m_rectPool;
					Tools::PointerPool<Node> m_nodePool;

#ifdef PTHREADS
					pthread_rwlock_t m_rwLock;
#else
					bool m_rwLock;
#endif

					friend class Node;
					friend class PointBindLine;
					friend class NVDataPublish::Features::LayerBulkLoader;
					friend class NVDataPublish::Network::PartitionMap;
					friend class NVDataPublish::Network::AdjacencyList;

				}; // StaticRTree

				ISpatialIndex* returnStaticRTree(IStorageManager& sm, Tools::PropertySet& ps);
				ISpatialIndex* createNewStaticRTree(
					IStorageManager& sm,
					id_type& indexIdentifier,
					size_t condenseStrategy
					);
				ISpatialIndex* createAndBulkLoadNewStaticRTree(
					BulkLoadMethod m,
					IDataStream& stream,
					IStorageManager& sm,
					NVDataPublish::Features::CondenseData& cd,
					id_type& indexIdentifier,
					size_t condenseStrategy);
				ISpatialIndex* loadStaticRTree(IStorageManager& sm, id_type indexIdentifier);
			}
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_STATIC_RTREE_H_
