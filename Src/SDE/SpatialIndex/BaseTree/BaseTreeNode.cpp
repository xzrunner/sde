#include "BaseTreeNode.h"
#include "../Utility/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex;

//
// Tools::IObject interface
//

Tools::IObject* BaseTreeNode::clone()
{
	throw Tools::NotSupportedException("IObject::clone should never be called.");
}

//
// Tools::ISerializable interface
//

size_t BaseTreeNode::getByteArraySize() const
{
	return	
		(
		sizeof(size_t) +	// NodeType
		sizeof(size_t) +	// m_level
		sizeof(size_t) +	// m_children
		(m_children * (DIMENSION * sizeof(double) * 2 + sizeof(id_type) + sizeof(size_t))) +
		m_totalDataLength +
		(2 * DIMENSION * sizeof(double))
		);
}


void BaseTreeNode::loadFromByteArray(const byte* ptr)
{
	m_nodeMBR.makeInfinite();

	// skip the node type information, it is not needed.
	ptr += sizeof(size_t);

	memcpy(&m_level, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(&m_children, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

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


void BaseTreeNode::storeToByteArray(byte** data, size_t& len) const
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
// SpatialIndex::IEntry interface
//

IS_SDE::id_type BaseTreeNode::getIdentifier() const
{
	return m_identifier;
}


void BaseTreeNode::getShape(IS_SDE::IShape** out) const
{
	*out = new Rect(m_nodeMBR);
}

//
// SpatialIndex::INode interface
//

size_t BaseTreeNode::getChildrenCount() const
{
	return m_children;
}


IS_SDE::id_type BaseTreeNode::getChildIdentifier(size_t index) const
{
	if (index < 0 || index >= m_children) 
		throw Tools::IndexOutOfBoundsException(index);

	return m_pIdentifier[index];
}

void BaseTreeNode::setChildIdentifier(size_t index, id_type id)
{
	if (index < 0 || index >= m_children) 
		throw Tools::IndexOutOfBoundsException(index);

	 m_pIdentifier[index] = id;
}

void BaseTreeNode::getChildShape(id_type id, IS_SDE::IShape** out) const
{
	if (isIndex())
	{
		*out = NULL;
		return;
	}

	//assert(id < m_children);
	//Data d(m_pDataLength[id], m_pData[id], *m_ptrMBR[id], m_pIdentifier[id]);
	//d.getShape(out);
	//return;

	for (size_t i = 0; i < m_capacity; ++i)
	{
		if (m_pIdentifier[i] == id)
		{
			Data d(m_pDataLength[i], m_pData[i], *m_ptrMBR[i], id);
			d.getShape(out);
			return;
		}
	}

	*out = NULL;
	return;
}

void BaseTreeNode::getChildShape(size_t index, IShape** out)  const
{
	if (isIndex() || index >= m_capacity)
	{
		*out = NULL;
		return;
	}

	Data d(m_pDataLength[index], m_pData[index], *m_ptrMBR[index], m_pIdentifier[index]);
	d.getShape(out);
}

size_t BaseTreeNode::getLevel() const
{
	return m_level;
}

bool BaseTreeNode::isIndex() const 
{ 
	throw Tools::IllegalStateException("This should never be called. ");
}

bool BaseTreeNode::isLeaf() const 
{ 
	throw Tools::IllegalStateException("This should never be called. ");
}

size_t BaseTreeNode::getNodeType() const 
{ 
	throw Tools::IllegalStateException("This should never be called. ");
}

RectPtr BaseTreeNode::getRectPtr() const 
{ 
	throw Tools::IllegalStateException("This should never be called. ");
}

//
// Internal
//

BaseTreeNode::BaseTreeNode() :
	m_level(0),
	m_identifier(-1),
	m_children(0),
	m_capacity(0),
	m_pData(0),
	m_ptrMBR(0),
	m_pIdentifier(0),
	m_pDataLength(0),
	m_totalDataLength(0)
{
}


BaseTreeNode::BaseTreeNode(IS_SDE::id_type id, size_t level, size_t capacity) :
	m_level(level),
	m_identifier(id),
	m_children(0),
	m_capacity(capacity),
	m_pData(0),
	m_ptrMBR(0),
	m_pIdentifier(0),
	m_pDataLength(0),
	m_totalDataLength(0)
{
	m_nodeMBR.makeInfinite();

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


BaseTreeNode::~BaseTreeNode()
{
	if (m_pData != 0)
	{
		for (size_t cChild = 0; cChild < m_children; cChild++)
		{
			if (m_pData[cChild] != 0) delete[] m_pData[cChild];
		}

		delete[] m_pData;
	}

	delete[] m_pDataLength;
	delete[] m_ptrMBR;
	delete[] m_pIdentifier;
}

void BaseTreeNode::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id) 
{ 
	assert(m_children < m_capacity);

	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = getRectPtr();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	m_totalDataLength += dataLength;
	++m_children;
}

void BaseTreeNode::deleteEntry(size_t index) 
{ 
	throw Tools::IllegalStateException("This should never be called. ");
}

void BaseTreeNode::clearAllEntry(bool invalidateRegion/* = true*/)
{
	throw Tools::IllegalStateException("This should never be called. ");
}
