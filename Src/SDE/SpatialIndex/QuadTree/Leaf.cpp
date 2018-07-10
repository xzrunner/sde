#include "QuadTree.h"
#include "Leaf.h"
#include "Index.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::QuadTree;

Leaf::~Leaf()
{
}

void Leaf::deleteDataWithNotCondenseTree(size_t index)
{
	if (index >= m_children)
	{
		throw Tools::IllegalArgumentException(
			"Leaf::deleteDataWithNotCondenseTree: over the bound. "
			);
	}

	deleteEntry(index);
}

Leaf::Leaf(IS_SDE::SpatialIndex::QuadTree::QuadTree* pTree, id_type id)
	: Node(pTree, id, 0, pTree->m_leafCapacity)
{
}

Leaf::Leaf(IS_SDE::SpatialIndex::QuadTree::QuadTree* pTree, id_type id, size_t capacity)
	: Node(pTree, id, 0, capacity)
{
}

void Leaf::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	assert(m_nodeMBR.intersectsRect(mbr));
	BaseTreeNode::insertEntry(dataLength, pData, mbr, id);
	++m_pQTree->m_stats.m_data;
}

void Leaf::deleteEntry(size_t index, bool invalidateRegion /* = false */)
{
	Node::deleteEntry(index);
	--m_pQTree->m_stats.m_data;
}

void Leaf::chooseSubtree(const Rect& mbr, std::vector<id_type>& leaves)
{
	leaves.push_back(m_identifier);
}

void Leaf::insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	if (m_children < m_capacity)
	{
		insertEntry(dataLength, pData, mbr, id);
		m_pQTree->writeNode(this);
	}
	else
	{
		std::vector<NodePtr> children;
		split(children);
		size_t duplicate = 0;
		for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
		{
			if (children.at(i)->m_nodeMBR.intersectsRect(mbr))
			{
				children.at(i)->insertEntry(dataLength, pData, mbr, id);
				++duplicate;
			}
		}
		if (duplicate != 0)
		{
			++m_pQTree->m_stats.m_duplicateObj;
			m_pQTree->m_stats.m_duplicateTotal += duplicate;
		}

		for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
			m_pQTree->writeNode(children.at(i).get());

		m_pQTree->deleteNode(this);

		NodePtr ptrR = m_pQTree->m_indexPool.acquire();
		if (ptrR.get() == 0)
		{
			ptrR = NodePtr(new Index(m_pQTree, m_identifier, m_level), &(m_pQTree->m_indexPool));
		}
		else
		{
			//ptrR->m_pQTree = m_pQTree;
			ptrR->m_identifier = m_identifier;
			ptrR->m_level = m_level;
			ptrR->m_nodeMBR = m_nodeMBR;
		}

		for	(size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
			ptrR->insertEntry(0, 0, children.at(i)->m_nodeMBR, children.at(i)->m_identifier);

		m_pQTree->writeNode(ptrR.get());
	}
}

void Leaf::deleteData(id_type id)
{
	size_t child;

	for (child = 0; child < m_children; child++)
	{
		if (m_pIdentifier[child] == id) 
			break;
	}

	if (child != m_children)
	{
		deleteEntry(child);
		m_pQTree->writeNode(this);

		// overflow node need not to condense.
		if (getNodeType() == NT_Leaf)
			condenseTree();
	}
}

// todo: If PoolPointer used in vector will have some problems ? 
//		 use id_type or Node*
void Leaf::split(std::vector<NodePtr>& children)
{
	for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
	{
		id_type childID = m_identifier * 4 + i + 1;
		NodePtr child = m_pQTree->m_leafPool.acquire();
		if (child.get() == 0)
			child = NodePtr(new Leaf(m_pQTree, childID), &(m_pQTree->m_leafPool));
		child->m_level = m_level + 1;
		child->m_identifier = childID;
		setChildRegion((IndexChildPos)i, &child->m_nodeMBR);
		children.push_back(child);
	}

	for (size_t i = 0; i < m_children; ++i)
	{
		size_t duplicate = 0;
		for (size_t j = 0; j < INDEX_CHILDREN_SIZE; ++j)
		{
			if (children.at(j)->m_nodeMBR.intersectsRect(*m_ptrMBR[i]))
			{
				size_t len = m_pDataLength[i];
				byte* data = new byte[m_pDataLength[i]];
				memcpy(data, m_pData[i], len);

				children.at(j)->insertEntry(len, data, *m_ptrMBR[i], m_pIdentifier[i]);
				++duplicate;
			}
		}

		if (duplicate != 0)
		{
			++m_pQTree->m_stats.m_duplicateObj;
			m_pQTree->m_stats.m_duplicateTotal += duplicate;
		}
	}

	clearAllEntry();

	++m_pQTree->m_stats.m_split;
}

void Leaf::condenseTree()
{
	id_type cParent = Utility::getParentID(m_identifier);
	if (cParent != -1)
	{
		NodePtr ptrParent = m_pQTree->readNode(cParent);
		
		std::set<id_type> totChildObjID;
		for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
		{
			NodePtr ptrChild = m_pQTree->readNode(ptrParent->m_pIdentifier[i]);
			if (ptrChild->isIndex())
				return;

			for (size_t j = 0; j < ptrChild->getChildrenCount(); ++j)
				totChildObjID.insert(ptrChild->m_pIdentifier[j]);
		}

		if (totChildObjID.size() <= m_capacity)
		{
			NodePtr newParent = m_pQTree->m_leafPool.acquire();
			if (newParent.get() == 0)
				newParent = NodePtr(new Leaf(m_pQTree, cParent), &(m_pQTree->m_leafPool));
			newParent->m_level		= ptrParent->m_level;
			newParent->m_identifier = cParent;
			newParent->m_nodeMBR	= ptrParent->m_nodeMBR;

			std::set<id_type> allObjsID;
			for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
			{
				NodePtr ptrChild = m_pQTree->readNode(ptrParent->m_pIdentifier[i]);
				assert(ptrChild->isLeaf());
				for (size_t j = 0; j < ptrChild->m_children; ++j)
				{
					std::pair<std::set<id_type>::iterator, bool> state = 
						allObjsID.insert(ptrChild->m_pIdentifier[j]);
					if (state.second)
					{
						size_t len = ptrChild->m_pDataLength[j];
						byte* data = new byte[len];
						memcpy(data, ptrChild->m_pData[j], len);

						newParent->insertEntry(len, data, *ptrChild->m_ptrMBR[j], ptrChild->m_pIdentifier[j]);
					}
				}
				ptrChild->clearAllEntry();
				m_pQTree->deleteNode(ptrChild.get());
			}
			m_pQTree->deleteNode(ptrParent.get());
			m_pQTree->writeNode(newParent.get());
			static_cast<Leaf*>(newParent.get())->condenseTree();
		}
	}
}

Overflow::~Overflow()
{
}

Overflow::Overflow(IS_SDE::SpatialIndex::QuadTree::QuadTree* pTree, id_type id)
	: Leaf(pTree, id, pTree->m_overflowCapacity)
{
}

void Overflow::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	BaseTreeNode::insertEntry(dataLength, pData, mbr, id);
	++m_pQTree->m_stats.m_data;
	m_nodeMBR.combineRect(mbr);
}

void Overflow::deleteEntry(size_t index, bool invalidateRegion/* = false*/)
{
	Node::deleteEntry(index, true);
}

void Overflow::insertData(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	if (m_children < m_capacity)
	{
		insertEntry(dataLength, pData, mbr, id);
		m_pQTree->writeNode(this);
	}
	else
	{
		m_nodeMBR.combineRect(mbr);
		m_pQTree->enlargeToContainOverflowNode(m_nodeMBR);

		for (size_t i = 0; i < m_children; ++i)
		{
#ifndef NDEBUG
			NodePtr root = m_pQTree->readNode(m_pQTree->m_rootID);
			if (!root->m_nodeMBR.containsRect(m_nodeMBR))
				throw Tools::IllegalStateException(
				"Overflow::insertData: Enlarged root's scope not contain overflow. "
				);
#endif
		
			size_t len = m_pDataLength[i];
			byte* pData = new byte[len];
			memcpy(pData, m_pData[i], len);
			id_type tmpNodeID;
			m_pQTree->insertData_impl(m_pDataLength[i], pData, *m_ptrMBR[i], m_pIdentifier[i], &tmpNodeID);
		}

		// clear all entry
		clearAllEntry(true);

		m_pQTree->writeNode(this);

		id_type tmpNodeID;
		m_pQTree->insertData_impl(dataLength, pData, mbr, id, &tmpNodeID);
	}
}
