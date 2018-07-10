#include "Node.h"
#include "NVRTree.h"
#include "Leaf.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::NVRTree;

Leaf::~Leaf()
{
}

Leaf::Leaf(IS_SDE::SpatialIndex::NVRTree::NVRTree* pTree, id_type id)
	: Node(pTree, id, 0, pTree->m_leafCapacity)
{
}

bool Leaf::findLeafPath(id_type nodeID, const Rect& nodeMBR, 
						std::stack<std::pair<id_type, size_t> >* pathBuffer)
{
	if (m_identifier == nodeID)
		return true;
	else
		return false;
}