#include "Leaf.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW;

Leaf::Leaf(const Rect& scope, const std::vector<size_t>& singleALPageIDs)
	: Node(scope), m_singleALPageIDs(singleALPageIDs)
{
}

void Leaf::postorderTraversal(std::vector<Node*>& grids)
{
	grids.push_back(this);
}