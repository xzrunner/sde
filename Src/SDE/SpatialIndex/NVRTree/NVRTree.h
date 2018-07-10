#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_NVRTREE_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_NVRTREE_H_
#include "../BaseTree/BaseTree.h"
#include "Statistics.h"
#include "Utility.h"
#include "PointerPoolNode.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class LayerStream;
		}

		namespace Arrangement
		{
			class PartitionMap;
		}
	}

	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class Statistics;

			class NVRTree : public BaseTree<Node, Statistics>
			{
			public:
				NVRTree(IStorageManager&, Tools::PropertySet&);
				// String                   Value		Description
				// ----------------------------------------------
				// IndexIdentifier			VT_LONG		If specified an existing index will be openened from the supplied
				//										storage	manager with the given index id. Behaviour is unspecified
				//										if the index id or the storage manager are incorrect.
				// IndexCapacity            VT_ULONG	The index node capacity. Default is 100.
				// LeafCapactiy             VT_ULONG	The leaf node capacity. Default is 100.
				// FillFactor               VT_DOUBLE	The fill factor. Default is 70%
				// DataType					VT_LONG		One of Point, Line or Polygon
				// IndexPoolCapacity        VT_LONG		Default is 100
				// LeafPoolCapacity         VT_LONG		Default is 100
				// RectPoolCapacity			VT_LONG		Default is 1000
				// PointPoolCapacity        VT_LONG		Default is 500

				virtual ~NVRTree();

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

			private:
				virtual void initNew(Tools::PropertySet&);
				virtual void initOld(Tools::PropertySet& ps);
				virtual void storeHeader();
				virtual void loadHeader();

				id_type writeNode(Node*);
				NodePtr readNode(id_type id);
				void deleteNode(Node*);

				virtual void rangeQuery(RangeQueryType type, const IShape& query, IVisitor&, bool);

				void findLeafPath(id_type nodeID, const Rect& nodeMBR, std::stack<std::pair<id_type, size_t> >* pathBuffer);
				void adjustTree(const NodePtr& n, const Rect& originalMBR);
				void condenseTree(const NodePtr& n);

			private:
				double m_fillFactor;

				friend class Node;
				friend class Index;
				friend class BulkLoader;
				friend class NVDataPublish::Arrangement::LineMgr;
				friend std::ostream& operator << (std::ostream& os, const NVRTree& t);

				friend class NVDataPublish::Features::LayerStream;
				friend class NVDataPublish::Arrangement::PartitionMap;

			}; // NVRTree

			ISpatialIndex* returnNVRTree(IStorageManager& sm, Tools::PropertySet& ps);
			ISpatialIndex* createNewNVRTree(
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			ISpatialIndex* createAndBulkLoadNewNVRTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				id_type& indexIdentifier);
			ISpatialIndex* loadNVRTree(IStorageManager& sm, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_NVRTREE_H_
