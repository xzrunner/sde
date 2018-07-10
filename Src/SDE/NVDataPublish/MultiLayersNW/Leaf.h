#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_LEAF_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_LEAF_H_
#include "Node.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			class Leaf : public Node
			{
			public:
				Leaf(const Rect& scope, const std::vector<size_t>& singleALPageIDs);

				virtual bool isLeaf() const { return true; }

				virtual void postorderTraversal(std::vector<Node*>& grids);

			private:
				std::vector<size_t> m_singleALPageIDs;

				friend class Publish;

			}; // Leaf
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_LEAF_H_
