#ifndef _IS_SDE_SPATIALINDEX_RTREE_LEAF_H_
#define _IS_SDE_SPATIALINDEX_RTREE_LEAF_H_

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace RTree
		{
			class Leaf : public Node
			{
			public:
				virtual ~Leaf();

			private:
				Leaf(RTree* pTree, id_type id);

				virtual NodePtr chooseSubtree(const Rect& mbr, size_t level, std::stack<id_type>& pathBuffer);
				virtual NodePtr findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer);

				virtual void split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& left, NodePtr& right);

				virtual void deleteData(id_type id, std::stack<id_type>& pathBuffer);

				friend class RTree;
				friend class BulkLoader;
			}; // Leaf
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_LEAF_H_
