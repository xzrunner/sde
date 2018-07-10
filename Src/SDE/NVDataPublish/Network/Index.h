#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_INDEX_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_INDEX_H_
#include "Node.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class Index : public Node
			{
			public:
				Index(const Rect& scope) : Node(scope) {}
				virtual ~Index();

				virtual void split(PartitionMap& pm);
				virtual bool isLeaf() { return false; }
				virtual void queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const;
				virtual void setNodeCount();

			private:
				bool shouldSplitNode(const PartitionMap& al, const Rect& scope) const;

			private:
				// 0 1
				// 2 3
				Node* m_child[4];

				friend class PartitionMap;

			}; // Index
		}
	}	
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_INDEX_H_
