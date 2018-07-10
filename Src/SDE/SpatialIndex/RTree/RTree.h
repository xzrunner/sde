#ifndef _IS_SDE_SPATIALINDEX_RTREE_RTREE_H_
#define _IS_SDE_SPATIALINDEX_RTREE_RTREE_H_
#include "../BaseTree/BaseTree.h"
#include "Statistics.h"
#include "Utility.h"
#include "PointerPoolNode.h"

namespace IS_SDE
{
	namespace TEST
	{
		class SpatialIndexLoader;
	}

	namespace SpatialIndex
	{
		namespace RTree
		{
			class Statistics;

			class RTree : public BaseTree<Node, Statistics>
			{
				class NNEntry;

			public:
				RTree(IStorageManager&, Tools::PropertySet&);
				// String                   Value		Description
				// ----------------------------------------------
				// IndexIdentifier			VT_LONG		If specified an existing index will be openened from the supplied
				//										storage	manager with the given index id. Behaviour is unspecified
				//										if the index id or the storage manager are incorrect.
				// Dimension                VT_ULONG	Dimensionality of the data that will be inserted.
				// IndexCapacity            VT_ULONG	The index node capacity. Default is 100.
				// LeafCapactiy             VT_ULONG	The leaf node capacity. Default is 100.
				// FillFactor               VT_DOUBLE	The fill factor. Default is 70%
				// TreeVariant              VT_LONG		Can be one of Linear, Quadratic or Rstar. Default is Rstar
				// DataType					VT_LONG		One of Point, Line or Polygon
				// NearMinimumOverlapFactor VT_ULONG	Default is 32.
				// SplitDistributionFactor  VT_DOUBLE	Default is 0.4
				// ReinsertFactor           VT_DOUBLE	Default is 0.3
				// EnsureTightMBRs          VT_BOOL		Default is true
				// IndexPoolCapacity        VT_LONG		Default is 100
				// LeafPoolCapacity         VT_LONG		Default is 100
				// RectPoolCapacity			VT_LONG		Default is 1000
				// PointPoolCapacity        VT_LONG		Default is 500

				virtual ~RTree();

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
				virtual void initNew(Tools::PropertySet&);
				virtual void initOld(Tools::PropertySet& ps);
				virtual void storeHeader();
				virtual void loadHeader();

				virtual void insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type objID, id_type* nodeID);
				void insertData_impl(size_t dataLength, byte* pData, const Rect& mbr, id_type id, size_t level, byte* overflowTable);
				virtual bool deleteData_impl(const Rect& mbr, id_type id);

				id_type writeNode(Node*);
				NodePtr readNode(id_type id);
				void deleteNode(Node*);

				virtual void rangeQuery(RangeQueryType type, const IShape& query, IVisitor&, bool);

				void selfJoinQuery(id_type id1, id_type id2, const Rect& r, IVisitor& vis);

				RTreeVariant m_treeVariant;

				double m_fillFactor;

				size_t m_nearMinimumOverlapFactor;
				// The R*-Tree 'p' constant, for calculating nearly minimum overlap cost.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles', Section 4.1]

				double m_splitDistributionFactor;
				// The R*-Tree 'm' constant, for calculating spliting distributions.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles', Section 4.2]

				double m_reinsertFactor;
				// The R*-Tree 'p' constant, for removing entries at reinserts.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				//  for Points and Rectangles', Section 4.3]

				bool m_bTightMBRs;

				class NNEntry
				{
				public:
					id_type m_id;
					IEntry* m_pEntry;
					double m_minDist;

					NNEntry(id_type id, IEntry* e, double f) : m_id(id), m_pEntry(e), m_minDist(f) {}
					~NNEntry() {}

					struct ascending : public std::binary_function<NNEntry*, NNEntry*, bool>
					{
						bool operator()(const NNEntry* __x, const NNEntry* __y) const { return __x->m_minDist > __y->m_minDist; }
					};
				}; // NNEntry

				class NNComparator : public INearestNeighborComparator
				{
				public:
					double getMinimumDistance(const IShape& query, const IShape& entry)
					{
						return query.getMinimumDistance(entry);
					}

					double getMinimumDistance(const IShape& query, const IData& data)
					{
						IShape* pS;
						data.getShape(&pS);
						double ret = query.getMinimumDistance(*pS);
						delete pS;
						return ret;
					}
				}; // NNComparator

				class ValidateEntry
				{
				public:
					ValidateEntry(Rect& r, NodePtr& pNode) : m_parentMBR(r), m_pNode(pNode) {}

					Rect m_parentMBR;
					NodePtr m_pNode;
				}; // ValidateEntry

				friend class Node;
				friend class Leaf;
				friend class Index;
				friend class BulkLoader;

				friend std::ostream& operator<<(std::ostream& os, const RTree& t);

				friend class SpatialIndexLoader;
			}; // RTree

			ISpatialIndex* returnRTree(IStorageManager& sm, Tools::PropertySet& ps);
			ISpatialIndex* createNewRTree(
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				RTreeVariant rv,
				id_type& indexIdentifier
				);
			ISpatialIndex* createAndBulkLoadNewRTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				RTreeVariant rv,
				id_type& indexIdentifier
				);
			ISpatialIndex* loadRTree(IStorageManager& in, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_RTREE_H_