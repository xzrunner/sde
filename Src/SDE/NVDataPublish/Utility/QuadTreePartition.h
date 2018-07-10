#ifndef _IS_SDE_NVDATAPUBLISH_UTILITY_QUADTREE_PARTITION_H_
#define _IS_SDE_NVDATAPUBLISH_UTILITY_QUADTREE_PARTITION_H_
#include "../Base.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SNode;
			class SIndex;
			class SLeaf;
		}
	}

	namespace NVDataPublish
	{
		namespace Utility
		{
			typedef SimulateNavigate::Network::SNode	SrcNode;
			typedef SimulateNavigate::Network::SIndex	SrcIndex;
			typedef SimulateNavigate::Network::SLeaf	SrcLeaf;

			class QPNode
			{
			public:
				QPNode(SrcNode* src);
				QPNode(const Rect& r, const std::vector<size_t>& dataPageIDs);
				~QPNode();

				bool isLeaf() const { return m_child[0] == NULL; }
				void reconstruct(IStorageManager* sm, size_t capacity);

				const QPNode* getChild(size_t pos) const { 
					assert(pos < 4 && pos >= 0);
					return m_child[pos]; 
				}

				const std::vector<size_t>& getSingleALPageIDs() const { return m_singleALPageIDs; }

			private:
				void condense();
				void split(size_t capacity, const std::vector<size_t>& dataPageIDs);

			private:
				QPNode* m_child[4];
				SrcNode* m_srcNode;
				std::vector<MapPos2D> m_allTopoNodePos;
				Rect m_scope;

				std::vector<size_t> m_singleALPageIDs;

			}; // QPNode
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_UTILITY_QUADTREE_PARTITION_H_