#ifndef _IS_SDE_SPATIALINDEX_RTREE_INDEX_H_
#define _IS_SDE_SPATIALINDEX_RTREE_INDEX_H_

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace RTree
		{
			class Index : public Node
			{
			public:
				virtual ~Index();

			private:
				Index(RTree* pTree, id_type id, size_t level);

				virtual NodePtr chooseSubtree(const Rect& mbr, size_t level, std::stack<id_type>& pathBuffer);
				virtual NodePtr findLeaf(const Rect& mbr, id_type id, std::stack<id_type>& pathBuffer);

				virtual void split(size_t dataLength, byte* pData, const Rect& mbr, id_type id, NodePtr& left, NodePtr& right);

				size_t findLeastEnlargement(const Rect&) const;
				size_t findLeastOverlap(const Rect&) const;

				void adjustTree(Node*, std::stack<id_type>&);
				void adjustTree(Node*, Node*, std::stack<id_type>&, byte* overflowTable);

				class OverlapEntry
				{
				public:
					size_t m_index;
					double m_enlargement;
					RectPtr m_original;
					RectPtr m_combined;
					double m_oa;
					double m_ca;

					static int compareEntries(const void* pv1, const void* pv2)
					{
						OverlapEntry* pe1 = * (OverlapEntry**) pv1;
						OverlapEntry* pe2 = * (OverlapEntry**) pv2;

						if (pe1->m_enlargement < pe2->m_enlargement) return -1;
						if (pe1->m_enlargement > pe2->m_enlargement) return 1;
						return 0;
					}
				}; // OverlapEntry

				friend class RTree;
				friend class Node;
				friend class BulkLoader;
			}; // Index
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_INDEX_H_

