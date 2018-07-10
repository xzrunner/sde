#include "Index.h"
#include "Leaf.h"
#include "../Utility/QuadTreePartition.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW;

typedef NVDataPublish::Utility::QPNode QPNode;

Index::Index(const QPNode* src, const Rect& scope)
	: Node(scope)
{
	assert(!src->isLeaf());
	Rect childScope(scope);
	for (size_t i = 0; i < 4; ++i)
	{
		Tools::SpatialMath::getRectChildGrid(scope, i, childScope);

		const QPNode* child = src->getChild(i);
		if (child->isLeaf())
			m_child[i] = new Leaf(childScope, child->getSingleALPageIDs());
		else
			m_child[i] = new Index(child, childScope);
	}
}

void Index::postorderTraversal(std::vector<Node*>& grids)
{
	for (size_t i = 0; i < 4; ++i)
		m_child[i]->postorderTraversal(grids);
	grids.push_back(this);
}