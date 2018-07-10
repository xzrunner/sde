#ifndef _IS_SDE_SPATIALINDEX_QUADTREE_NODE_H_
#define _IS_SDE_SPATIALINDEX_QUADTREE_NODE_H_
#include "../BaseTree/BaseTreeNode.h"
#include "Utility.h"
#include "Statistics.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Node : public BaseTreeNode
			{
			private:
				Node();
				Node(QuadTree* pTree, id_type id, size_t level, size_t capacity);

				virtual Node& operator=(const Node&);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index, bool invalidateRegion = false);
				virtual void clearAllEntry(bool invalidateRegion = false);

				virtual void chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves);

				void setChildRegion(IndexChildPos index, Rect* rect) const; 

				virtual RectPtr getRectPtr() const;

				QuadTree* m_pQTree;
					// Parent of all nodes.

				friend class QuadTree;
				friend class Leaf;
				friend class Index;
				friend class Overflow;
				friend class BulkLoader;
				friend class Tools::PointerPool<Node>;
			};	// Node

			typedef Tools::PoolPointer<Node> NodePtr;
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUADTREE_NODE_H_