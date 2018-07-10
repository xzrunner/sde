#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_NODE_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_NODE_H_
#include "../Base.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			typedef size_t SINGLE_NODE_ID;
			typedef size_t MULTI_NODE_ID;

			class Node
			{
			public:
				Node(const Rect& scope) : m_scope(scope) {}
				virtual ~Node() {}

				virtual bool isLeaf() const = 0;

				virtual void postorderTraversal(std::vector<Node*>& grids) = 0;

			private:
				Rect m_scope;

				std::map<SINGLE_NODE_ID, MULTI_NODE_ID> m_transEnterGridNodeIDs;

				id_type m_startPage;
				size_t m_pageCount;

				friend class Publish;
				friend class MidGraph;
				friend class PartitionMap;
				friend class EnterGridNodesMapping;

			}; // Node
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_NODE_H_
