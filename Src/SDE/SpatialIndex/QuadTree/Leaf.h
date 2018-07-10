#ifndef _IS_SDE_SPATIALINDEX_QUAD_TREE_LEAF_H_
#define _IS_SDE_SPATIALINDEX_QUAD_TREE_LEAF_H_

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Leaf : public Node
			{
			public:
				virtual ~Leaf();

				virtual bool isIndex() const { return false; }
				virtual bool isLeaf() const { return true; }

				virtual size_t getNodeType() const { return NT_Leaf; }

				void deleteDataWithNotCondenseTree(size_t index);

			private:
				Leaf(QuadTree* pTree, id_type id);
				Leaf(QuadTree* pTree, id_type id, size_t capacity);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index, bool invalidateRegion = false);

				virtual void chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves);

				virtual void insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteData(id_type id);

				void split(std::vector<NodePtr>& children);
				void condenseTree();

				friend class QuadTree;
				friend class Overflow;
				friend class BulkLoader;
			}; // Leaf

			class Overflow : public Leaf
			{
			public:
				virtual ~Overflow();

				virtual size_t getNodeType() const { return NT_Overflow; }

			private:
				Overflow(QuadTree* pTree, id_type id);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index, bool invalidateRegion = false);

				virtual void insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id);

				friend class QuadTree;
			};	// Overflow
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUAD_TREE_LEAF_H_
