#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_NODE_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_NODE_H_
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class PartitionMap;

			class Node
			{
			public:
				Node(const Rect& scope) : m_scope(scope), m_nodeCount(0) {}
				virtual ~Node() {}

				virtual void split(PartitionMap& pm) = 0;
				virtual bool isLeaf() = 0;
				virtual void queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const = 0;
				virtual void setNodeCount() = 0;

			public:
				static const size_t PM_NODE_ID_SIZE		= 4;
				static const size_t PM_NODE_COUNT_SIZE	= 4;

				static const size_t GRID_SIZE_BIT	= static_cast<size_t>(8 * 2.5);
				static const size_t OFFSET_SIZE_BIT	= static_cast<size_t>(8 * 1.5);

			private:
				Rect m_scope;
				size_t m_nodeCount;

				friend class Publish;
				friend class PartitionMap;
				friend class AdjacencyList;
				friend class Leaf;
				friend class Index;

			}; // Node
		}
	}	
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_NODE_H_
