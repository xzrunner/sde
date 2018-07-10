#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_INDEX_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_INDEX_H_
#include "Node.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Utility
		{
			class QPNode;
		}

		namespace MLNW
		{
			class Index : public Node
			{
			public:
				Index(const Utility::QPNode* src, const Rect& scope);

				virtual bool isLeaf() const { return false; }

				virtual void postorderTraversal(std::vector<Node*>& grids);

			private:
				Node* m_child[4];

				friend class Publish;
				friend class PartitionMap;
				friend class EnterGridNodesMapping;

			}; // Index
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_INDEX_H_
