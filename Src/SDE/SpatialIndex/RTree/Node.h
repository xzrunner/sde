#ifndef _IS_SDE_SPATIALINDEX_RTREE_NODE_H_
#define _IS_SDE_SPATIALINDEX_RTREE_NODE_H_
#include "../BaseTree/BaseTreeNode.h"
#include "Statistics.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace RTree
		{
			class Node;
			typedef Tools::PoolPointer<Node> NodePtr;

			class Node : public BaseTreeNode
			{
			public:
				//
				// SpatialIndex::INode interface
				//
				virtual bool isIndex() const;
				virtual bool isLeaf() const;

				virtual size_t getNodeType() const;

			private:
				Node();
				Node(RTree* pTree, id_type id, size_t level, size_t capacity);

				virtual Node& operator=(const Node&);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index);
				virtual void clearAllEntry(bool invalidateRegion = true);

				virtual bool insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer, byte* overflowTable);
				virtual void reinsertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& reinsert, std::vector<size_t>& keep);

				virtual void rtreeSplit(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& group1, std::vector<size_t>& group2);
				virtual void rstarSplit(size_t dataLength, byte* pData, const Rect& mbr, id_type id, std::vector<size_t>& group1, std::vector<size_t>& group2);

				virtual void pickSeeds(size_t& index1, size_t& index2);

				virtual void condenseTree(std::stack<NodePtr>& toReinsert, std::stack<id_type>& pathBuffer, NodePtr& ptrThis);

				virtual NodePtr chooseSubtree(const Rect& mbr, size_t level, std::stack<id_type>& pathBuffer);
				virtual NodePtr findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer);

				virtual void split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& left, NodePtr& right);

				virtual RectPtr getRectPtr() const;

				RTree* m_pRTree;
					// Parent of all nodes.

				class RstarSplitEntry
				{
				public:
					Rect* m_pRect;
					size_t m_index;
					size_t m_sortDim;

					RstarSplitEntry(Rect* pr, size_t index, size_t dimension) :
					m_pRect(pr), m_index(index), m_sortDim(dimension) {}

					static int compareLow(const void* pv1, const void* pv2)
					{
						RstarSplitEntry* pe1 = * (RstarSplitEntry**) pv1;
						RstarSplitEntry* pe2 = * (RstarSplitEntry**) pv2;

						assert(pe1->m_sortDim == pe2->m_sortDim);

						if (pe1->m_pRect->m_pLow[pe1->m_sortDim] < pe2->m_pRect->m_pLow[pe2->m_sortDim]) return -1;
						if (pe1->m_pRect->m_pLow[pe1->m_sortDim] > pe2->m_pRect->m_pLow[pe2->m_sortDim]) return 1;
						return 0;
					}

					static int compareHigh(const void* pv1, const void* pv2)
					{
						RstarSplitEntry* pe1 = * (RstarSplitEntry**) pv1;
						RstarSplitEntry* pe2 = * (RstarSplitEntry**) pv2;

						assert(pe1->m_sortDim == pe2->m_sortDim);

						if (pe1->m_pRect->m_pHigh[pe1->m_sortDim] < pe2->m_pRect->m_pHigh[pe2->m_sortDim]) return -1;
						if (pe1->m_pRect->m_pHigh[pe1->m_sortDim] > pe2->m_pRect->m_pHigh[pe2->m_sortDim]) return 1;
						return 0;
					}
				}; // RstarSplitEntry

				class ReinsertEntry
				{
				public:
					size_t m_index;
					double m_dist;

					ReinsertEntry(size_t index, double dist) : m_index(index), m_dist(dist) {}

					static int compareReinsertEntry(const void* pv1, const void* pv2)
					{
						ReinsertEntry* pe1 = * (ReinsertEntry**) pv1;
						ReinsertEntry* pe2 = * (ReinsertEntry**) pv2;

						if (pe1->m_dist < pe2->m_dist) return -1;
						if (pe1->m_dist > pe2->m_dist) return 1;
						return 0;
					}
				}; // ReinsertEntry

				// Needed to access protected members without having to cast from Node.
				// It is more efficient than using member functions to access protected members.
				friend class RTree;
				friend class Leaf;
				friend class Index;
				friend class Tools::PointerPool<Node>;
				friend class BulkLoader;
			}; // Node
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_NODE_H_

