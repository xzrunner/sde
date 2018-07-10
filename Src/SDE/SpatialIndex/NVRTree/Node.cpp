#include "Node.h"
#include "NVRTree.h"
#include "../../BasicType/Point.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::NVRTree;

//
// Tools::ISerializable interface
//

size_t Node::getByteArraySize() const
{
	return	
		(
		sizeof(size_t) +	// NodeType
		sizeof(size_t) +	// m_level
		sizeof(size_t) +	// m_children
		sizeof(size_t) +	// m_capacity
		(m_children * (DIMENSION * sizeof(double) * 2 + sizeof(id_type) + sizeof(size_t))) +
		m_totalDataLength +
		(2 * DIMENSION * sizeof(double))
		);
}


void Node::loadFromByteArray(const byte* ptr)
{
	m_nodeMBR.makeInfinite();

	// skip the node type information, it is not needed.
	ptr += sizeof(size_t);

	memcpy(&m_level, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(&m_children, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	// debug todo: 
	bool bChangeCapatity = true;

	memcpy(&m_capacity, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	if (bChangeCapatity)
	{
		delete[] m_pDataLength;
		delete[] m_pData;
		delete[] m_ptrMBR;
		delete[] m_pIdentifier;			

		try
		{
			m_pDataLength = new size_t[m_capacity + 1];
			m_pData = new byte*[m_capacity + 1];
			m_ptrMBR = new RectPtr[m_capacity + 1];
			m_pIdentifier = new id_type[m_capacity + 1];
		}
		catch (...)
		{
			delete[] m_pDataLength;
			delete[] m_pData;
			delete[] m_ptrMBR;
			delete[] m_pIdentifier;
			throw;
		}
	}

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		m_ptrMBR[cChild] = getRectPtr();
		(m_ptrMBR[cChild])->makeInfinite();

		memcpy(m_ptrMBR[cChild]->m_pLow, ptr, DIMENSION * sizeof(double));
		ptr += DIMENSION * sizeof(double);
		memcpy(m_ptrMBR[cChild]->m_pHigh, ptr, DIMENSION * sizeof(double));
		ptr += DIMENSION * sizeof(double);
		memcpy(&(m_pIdentifier[cChild]), ptr, sizeof(id_type));
		ptr += sizeof(id_type);

		memcpy(&(m_pDataLength[cChild]), ptr, sizeof(size_t));
		ptr += sizeof(size_t);

		if (m_pDataLength[cChild] > 0)
		{
			m_totalDataLength += m_pDataLength[cChild];
			m_pData[cChild] = new byte[m_pDataLength[cChild]];
			memcpy(m_pData[cChild], ptr, m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
		else
		{
			m_pData[cChild] = 0;
		}

		//m_nodeMBR.combineRect(*(m_ptrMBR[cChild]));
	}

	memcpy(m_nodeMBR.m_pLow, ptr, DIMENSION * sizeof(double));
	ptr += DIMENSION * sizeof(double);
	memcpy(m_nodeMBR.m_pHigh, ptr, DIMENSION * sizeof(double));
	//ptr += DIMENSION * sizeof(double);
}


void Node::storeToByteArray(byte** data, size_t& len) const
{
	len = getByteArraySize();

	*data = new byte[len];
	byte* ptr = *data;

	size_t nodeType = getNodeType();

	memcpy(ptr, &nodeType, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(ptr, &m_level, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(ptr, &m_children, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(ptr, &m_capacity, sizeof(size_t));
	ptr += sizeof(size_t);

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		memcpy(ptr, m_ptrMBR[cChild]->m_pLow, DIMENSION * sizeof(double));
		ptr += DIMENSION * sizeof(double);
		memcpy(ptr, m_ptrMBR[cChild]->m_pHigh, DIMENSION * sizeof(double));
		ptr += DIMENSION * sizeof(double);
		memcpy(ptr, &(m_pIdentifier[cChild]), sizeof(id_type));
		ptr += sizeof(id_type);

		memcpy(ptr, &(m_pDataLength[cChild]), sizeof(size_t));
		ptr += sizeof(size_t);

		if (m_pDataLength[cChild] > 0)
		{
			memcpy(ptr, m_pData[cChild], m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
	}

	// store the node MBR for efficiency. This increases the node size a little bit.
	memcpy(ptr, m_nodeMBR.m_pLow, DIMENSION * sizeof(double));
	ptr += DIMENSION * sizeof(double);
	memcpy(ptr, m_nodeMBR.m_pHigh, DIMENSION * sizeof(double));
	//ptr += DIMENSION * sizeof(double);

	assert(len == (ptr - *data) + DIMENSION * sizeof(double));
}

//
// SpatialIndex::INode interface
//

bool Node::isLeaf() const
{
	return (m_level == 0);
}

bool Node::isIndex() const
{
	return (m_level != 0);
}

size_t Node::getNodeType() const
{
	if (m_level == 0)
		return PersistentLeaf;
	else
		return PersistentIndex;
}

void Node::updateOneRoadToMuti(size_t index, const std::vector<std::pair<IShape*, bool> >& roads)
{
	assert(index >= 0 && index < m_children);
	assert(!roads.empty());

	m_totalDataLength -= m_pDataLength[index];
	if (m_pData[index] != 0) delete[] m_pData[index];

	byte* buffer;
	size_t dataLength;
	roads.front().first->storeToByteArray(&buffer, dataLength);

	RectPtr mbr = m_pNVRTree->m_rectPool.acquire();
	roads.front().first->getMBR(*mbr);
	if (roads.front().second)
		m_nodeMBR.combineRect(*mbr);

	m_pDataLength[index]	= dataLength;
	m_pData[index]			= buffer;
	m_ptrMBR[index]			= mbr;

	m_totalDataLength += dataLength;

	for (size_t i = 1; i < roads.size(); ++i)
	{
		byte* buffer;
		size_t dataLength;
		roads.at(i).first->storeToByteArray(&buffer, dataLength);

		RectPtr mbr = m_pNVRTree->m_rectPool.acquire();
		roads.at(i).first->getMBR(*mbr);
		if (roads.at(i).second)
			m_nodeMBR.combineRect(*mbr);

		id_type id = m_pNVRTree->nextObjID();

		if (m_children == m_capacity)
		{
			size_t newCapacity = m_capacity * 2;

			size_t* pDataLength;
			byte** pData;
			RectPtr* ptrMBR;
			id_type* pIdentifier;

			try
			{
				pDataLength = new size_t[newCapacity + 1];
				pData		= new byte*[newCapacity + 1];
				ptrMBR		= new RectPtr[newCapacity + 1];
				pIdentifier = new id_type[newCapacity + 1];
			}
			catch (...)
			{
				delete[] pDataLength;
				delete[] pData;
				delete[] ptrMBR;
				delete[] pIdentifier;
				throw;
			}

			for (size_t i = 0; i < m_capacity; ++i)
			{
				pDataLength[i]	= m_pDataLength[i];
				pData[i]		= m_pData[i];
				ptrMBR[i]		= m_ptrMBR[i];
				pIdentifier[i]	= m_pIdentifier[i];
			}

			delete[] m_pDataLength;
			delete[] m_pData;
			delete[] m_ptrMBR;
			delete[] m_pIdentifier;			

			m_pDataLength = pDataLength;
			m_pData = pData;
			m_ptrMBR = ptrMBR;
			m_pIdentifier = pIdentifier;

			m_capacity = newCapacity;
		}

		m_pDataLength[m_children]	= dataLength;
		m_pData[m_children]			= buffer;
		m_ptrMBR[m_children]		= getRectPtr();
		*(m_ptrMBR[m_children])		= *mbr;
		m_pIdentifier[m_children]	= id;

		m_totalDataLength += dataLength;
		++m_children;
	}
}

void Node::deleteRoads(std::vector<size_t>& indexes)
{
	sort(indexes.begin(), indexes.end(), std::greater<size_t>());

	bool bChangeMBR = false;
	for (size_t i = 0; i < indexes.size(); ++i)
	{
		if (m_nodeMBR.touchesRect(*m_ptrMBR[i]))
			bChangeMBR = true;
		deleteEntry(indexes.at(i));
	}

	if (bChangeMBR)
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

Node::Node() 
	: BaseTreeNode(), m_pNVRTree(NULL)
{
}

Node::Node(IS_SDE::SpatialIndex::NVRTree::NVRTree* pTree, IS_SDE::id_type id, size_t level, size_t capacity) 
	: BaseTreeNode(id, level, capacity), m_pNVRTree(pTree)
{
}

Node& Node::operator = (const Node& n)
{
	throw Tools::IllegalStateException("operator =: This should never be called.");
}

void Node::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	BaseTreeNode::insertEntry(dataLength, pData, mbr, id);
	m_nodeMBR.combineRect(mbr);
}

void Node::deleteEntry(size_t index)
{
	assert(index >= 0 && index < m_children);

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
}

void Node::clearAllEntry(bool invalidateRegion/* = true*/)
{
	throw Tools::NotSupportedException("Not support. ");
}

void Node::condenseTree(std::stack<std::pair<id_type, size_t> >* pathBuffer)
{
	if (pathBuffer->empty())
	{
		assert(m_identifier == m_pNVRTree->m_rootID);
	}
	else
	{
		std::pair<id_type, size_t> pos = pathBuffer->top(); pathBuffer->pop();
		NodePtr ptrParent = m_pNVRTree->readNode(pos.first);

		bool bChanged = ptrParent->m_nodeMBR.touchesRect(*ptrParent->m_ptrMBR[pos.second]);
		if (m_children == 0)
		{
			ptrParent.get()->deleteEntry(pos.second);
			m_pNVRTree->deleteNode(this);
		}
	
		if (bChanged)
		{
			for (size_t cDim = 0; cDim < ptrParent->m_nodeMBR.DIMENSION; cDim++)
			{
				ptrParent->m_nodeMBR.m_pLow[cDim] = Tools::DOUBLE_MAX;
				ptrParent->m_nodeMBR.m_pHigh[cDim] = -Tools::DOUBLE_MAX;

				for (size_t cChild = 0; cChild < ptrParent->m_children; cChild++)
				{
					ptrParent->m_nodeMBR.m_pLow[cDim] = std::min(ptrParent->m_nodeMBR.m_pLow[cDim], ptrParent->m_ptrMBR[cChild]->m_pLow[cDim]);
					ptrParent->m_nodeMBR.m_pHigh[cDim] = std::max(ptrParent->m_nodeMBR.m_pHigh[cDim], ptrParent->m_ptrMBR[cChild]->m_pHigh[cDim]);
				}
			}
		}
		
		m_pNVRTree->writeNode(ptrParent.get());

		ptrParent->condenseTree(pathBuffer);
	}
}

bool Node::findLeafPath(id_type nodeID, const Rect& nodeMBR, 
						std::stack<std::pair<id_type, size_t> >* pathBuffer)
{
	throw Tools::IllegalStateException("Node::findNodePath: should never be called. ");
}

RectPtr Node::getRectPtr() const
{
	return m_pNVRTree->m_rectPool.acquire();
}