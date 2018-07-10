#ifndef _IS_SDE_SPATIALINDEX_QUADTREE_QUADTREE_H_
#define _IS_SDE_SPATIALINDEX_QUADTREE_QUADTREE_H_
#include "../BaseTree/BaseTree.h"
#include "Statistics.h"
#include "PointerPoolNode.h"

namespace IS_SDE
{
	namespace TEST
	{
		class SpatialIndexLoader;
	}

	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Statistics;
			class Node;
			class ChangeAllNodeID;

			class QuadTree : public BaseTree<Node, Statistics>
			{
			public:
				QuadTree(IStorageManager&, Tools::PropertySet&);
				// String                   Value		Description
				// ----------------------------------------------
				// IndexIndentifier         VT_LONGLING	If specified an existing index will be openened from the supplied
				//										storage manager with the given index id. Behaviour is unspecified
				//										if the index id or the storage manager are incorrect.
				//										header id set the max of int64_t = 2^63 - 1
				// LeafCapacity             VT_ULONG	The leaf node capacity. Default is 100.
				// OverflowCapacity			VT_ULONG	The overflow node capacity. Default is 100.
				// IndexPoolCapacity        VT_LONG		Default is 100
				// LeafPoolCapacity         VT_LONG		Default is 100
				// RectPoolCapacity			VT_LONG		Default is 1000
				// PointPoolCapacity        VT_LONG		Default is 500

				virtual ~QuadTree();

				//
				// ISpatialIndex interface
				//
				virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator&);
				virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v);
				virtual void selfJoinQuery(const IShape& s, IVisitor& v);
				virtual bool countIntersectsQuery(const IShape& query, ICountVisitor& v);
				virtual void getIndexProperties(Tools::PropertySet& out) const;
				virtual bool isIndexValid();
				virtual void getStatistics(IStatistics** out) const;
//				virtual void checkStoreState(ICheckStoreStrategy& css);

			private:
				virtual void initNew(Tools::PropertySet& ps);
				virtual void initOld(Tools::PropertySet& ps);
				virtual void storeHeader();
				virtual void loadHeader();

				virtual void insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID);
				virtual bool deleteData_impl(const Rect& mbr, id_type id);

				virtual id_type writeNode(Node*);
				virtual NodePtr readNode(id_type id);
				virtual void deleteNode(Node*);

				virtual void rangeQuery(RangeQueryType type, const IShape& query, IVisitor&, bool);

				void enlargeToContainOverflowNode(const Rect& overflowScope);

			private:

				class ChangeAllNodeID;

				// ensure physical IDs are continuous
				// no matter how coding logical IDs
				class IDManager : public Tools::ISerializable
				{
				public:
					//
					// ISerializable interface
					//
					virtual size_t getByteArraySize() const;
					virtual void loadFromByteArray(const byte* data);
					virtual void storeToByteArray(byte** data, size_t& length) const;

					id_type getPhysicalIDToWrite(id_type logicalID) const;
					void updateTransTable(id_type logicalID, id_type physicalID);
					id_type transIDLogicalToPhysical(id_type logicalID, bool isDelete = false);

				private:
					std::map<id_type, id_type> m_mapIDLogicalToPhysical;

					friend class IS_SDE::SpatialIndex::QuadTree::ChangeAllNodeID;

				};	// class IDManager

			private:
				id_type m_overflowID;

				size_t m_overflowCapacity;

				IDManager m_IDManager;

				friend class Node;
				friend class Leaf;
				friend class Index;
				friend class Overflow;
				friend class BulkLoader;

				friend class IS_SDE::SpatialIndex::QuadTree::ChangeAllNodeID;

				friend class SpatialIndexLoader;
			};	// QuadTree

			class ChangeAllNodeID
			{
			public:
				static void changeAllNodeID(size_t enlargeTimes, QuadTree* pQuadTree);

			private:
				static id_type getNodeIDAfterRootEnlargeOneTime(id_type oldNodeID);
				static id_type getNewNodeID(id_type oldNodeID, size_t times);

			}; // ChangeAllNodeID

			ISpatialIndex* returnQuadTree(IStorageManager& sm, Tools::PropertySet& ps);
			ISpatialIndex* createNewQuadTree(
				IStorageManager& sm,
				double fillFactor,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			ISpatialIndex* createAndBulkLoadNewQuadTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			ISpatialIndex* loadQuadTree(IStorageManager& in, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUADTREE_QUADTREE_H_