#include "Node.h"
#include "QuadTree.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::QuadTree;

//
// Internal
//

Node::Node() 
	: BaseTreeNode(), m_pQTree(NULL)
{
}

Node::Node(IS_SDE::SpatialIndex::QuadTree::QuadTree* pTree, IS_SDE::id_type id, size_t level, size_t capacity) 
	: BaseTreeNode(id, level, capacity), m_pQTree(pTree)
{
}

Node& Node::operator=(const Node& n)
{
	throw Tools::IllegalStateException("operator =: This should never be called.");
}

void Node::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	throw Tools::IllegalStateException("This should never be called.");
}

void Node::deleteEntry(size_t index, bool invalidateRegion/* = false*/)
{
	assert(index >= 0 && index < m_children);

	// cache it, since I might need it for "touches" later.
	RectPtr ptrR = m_ptrMBR[index];

	m_totalDataLength -= m_pDataLength[index];
	if (m_pData[index] != 0) delete[] m_pData[index];

	if (m_children > 1 && index != m_children - 1)
	{
		m_pDataLength[index] = m_pDataLength[m_children - 1];
		m_pData[index] = m_pData[m_children - 1];
		m_ptrMBR[index] = m_ptrMBR[m_children - 1];
		m_pIdentifier[index] = m_pIdentifier[m_children - 1];
	}

	m_children--;

	// WARNING: index has now changed. Do not use it below here.

	if (invalidateRegion)
	{
		if (m_children == 0)
		{
			m_nodeMBR = m_pQTree->m_infiniteRect;
		}
		else if (m_nodeMBR.touchesRect(*ptrR))
		{
			for (size_t cDim = 0; cDim < m_nodeMBR.DIMENSION; cDim++)
			{
				m_nodeMBR.m_pLow[cDim] = Tools::DOUBLE_MAX;
				m_nodeMBR.m_pHigh[cDim] = -Tools::DOUBLE_MAX;

				for (size_t cChild = 0; cChild < m_children; cChild++)
				{
					m_nodeMBR.m_pLow[cDim] = std::min(m_nodeMBR.m_pLow[cDim], m_ptrMBR[cChild]->m_pLow[cDim]);
					m_nodeMBR.m_pHigh[cDim] = std::max(m_nodeMBR.m_pHigh[cDim], m_ptrMBR[cChild]->m_pHigh[cDim]);
				}
			}
		}
	}
}

void Node::clearAllEntry(bool invalidateRegion/* = false*/)
{
	for (size_t i = 0; i < m_children; ++i)
	{
		delete[] m_pData[i];
		m_pData[i] = NULL;
	}
	m_totalDataLength = 0;
	m_children = 0;
	if (invalidateRegion)
		m_nodeMBR = m_pQTree->m_infiniteRect;
}

void Node::chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves)
{
	throw Tools::IllegalStateException("Should never be called. ");
}

void Node::setChildRegion(IndexChildPos index, Rect* rect) const
{
	Tools::SpatialMath::getRectChildGrid(m_nodeMBR, index, *rect);
}

RectPtr Node::getRectPtr() const
{
	return m_pQTree->m_rectPool.acquire();
}