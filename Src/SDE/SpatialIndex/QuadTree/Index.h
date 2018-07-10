#ifndef _IS_SDE_SPATIALINDEX_QUAD_TREE_INDEX_H_
#define _IS_SDE_SPATIALINDEX_QUAD_TREE_INDEX_H_

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Index : public Node
			{
			public:
				virtual ~Index();

				virtual bool isIndex() const { return true; }
				virtual bool isLeaf() const { return false; }

				virtual size_t getNodeType() const { return NT_Index; }

			private:
				Index(QuadTree* pTree, id_type id, size_t level);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index);

				virtual void chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves);

				friend class QuadTree;
				friend class Node;
				friend class Leaf;
				friend class Overflow;
				friend class BulkLoader;
			}; // Index
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUAD_TREE_INDEX_H_

