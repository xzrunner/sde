#include "QuadTree.h"
#include "Index.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::QuadTree;

Index::~Index()
{
}

Index::Index(IS_SDE::SpatialIndex::QuadTree::QuadTree* pTree, id_type id, size_t level) 
	: Node(pTree, id, level, INDEX_CHILDREN_SIZE)
{
}

void Index::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	assert(m_nodeMBR.containsRect(mbr));
	BaseTreeNode::insertEntry(dataLength, pData, mbr, id);
}

void Index::deleteEntry(size_t index)
{
	throw Tools::IllegalStateException("Index::deleteEntry: This should never be called. ");
}

void Index::chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves)
{
	for(size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
	{
		if (m_ptrMBR[i]->intersectsRect(mbr))
		{
			NodePtr n = m_pQTree->readNode(m_pIdentifier[i]);
			n->chooseSubtree(mbr, leaves);
		}
	}
}
