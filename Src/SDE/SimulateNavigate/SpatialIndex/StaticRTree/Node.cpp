#include "Node.h"
#include "Utility.h"
#include "StaticRTree.h"
#include "../../../NVDataPublish/Features/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::SpatialIndex::StaticRTree;

//
// Tools::IObject interface
//

Tools::IObject* Node::clone()
{
	throw Tools::NotSupportedException("Node::clone should never be called.");
}

//
// Tools::ISerializable interface
//

size_t Node::getByteArraySize() const
{
	return
		CHILDREN_COUNT_SIZE		+	// m_children
		MBR_SIZE				+	// m_nodeMBR
		m_totalDataLength		+
		m_children * (
			MBR_SIZE					+	// m_ptrMBR
			EDGE_ID_SIZE				+	// m_pIdentifier
			EACH_CHILD_SIZE_SIZE			// m_pDataLength
		);
}


void Node::loadFromByteArray(const byte* ptr)
{
	m_nodeMBR.makeInfinite();

	memcpy(&m_children, ptr, CHILDREN_COUNT_SIZE);
	ptr += CHILDREN_COUNT_SIZE;

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		m_ptrMBR[cChild] = getRectPtr();
		(m_ptrMBR[cChild])->makeInfinite();

		readMBRFromByte(*m_ptrMBR[cChild].get(), ptr);

		m_pIdentifier[cChild] = 0;
		memcpy(&(m_pIdentifier[cChild]), ptr, EDGE_ID_SIZE);
		ptr += EDGE_ID_SIZE;

		m_pDataLength[cChild] = 0;
		memcpy(&(m_pDataLength[cChild]), ptr, EACH_CHILD_SIZE_SIZE);
		ptr += EACH_CHILD_SIZE_SIZE;

		if (m_pDataLength[cChild] > 0)
		{
			m_bLeaf = true;
			m_totalDataLength += m_pDataLength[cChild];
			m_pData[cChild] = new byte[m_pDataLength[cChild]];
			memcpy(m_pData[cChild], ptr, m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
		else
		{
			m_pData[cChild] = 0;
		}
	}

	readMBRFromByte(m_nodeMBR, ptr);
}


void Node::storeToByteArray(byte** data, size_t& len) const
{
	len = getByteArraySize();

	*data = new byte[len];
	byte* ptr = *data;

	memcpy(ptr, &m_children, CHILDREN_COUNT_SIZE);
	ptr += CHILDREN_COUNT_SIZE;

	for (size_t cChild = 0; cChild < m_children; cChild++)
	{
		writeMBRToByte(*m_ptrMBR[cChild].get(), ptr);

		memcpy(ptr, &(m_pIdentifier[cChild]), EDGE_ID_SIZE);
		ptr += EDGE_ID_SIZE;

		memcpy(ptr, &(m_pDataLength[cChild]), EACH_CHILD_SIZE_SIZE);
		ptr += EACH_CHILD_SIZE_SIZE;

		if (m_pDataLength[cChild] > 0)
		{
			m_bLeaf = true;
			memcpy(ptr, m_pData[cChild], m_pDataLength[cChild]);
			ptr += m_pDataLength[cChild];
		}
	}

	// store the node MBR for efficiency. This increases the node size a little bit.
	writeMBRToByte(m_nodeMBR, ptr);

	assert(len == ptr - *data);
}

//
// SpatialIndex::IEntry interface
//

IS_SDE::id_type Node::getIdentifier() const
{
	return m_identifier;
}

void Node::getShape(IS_SDE::IShape** out) const
{
	*out = new Rect(m_nodeMBR);
}

//
// SpatialIndex::INode interface
//

size_t Node::getChildrenCount() const
{
	return m_children;
}

IS_SDE::id_type Node::getChildIdentifier(size_t index) const
{
	if (index < 0 || index >= m_children) 
		throw Tools::IndexOutOfBoundsException(index);

	return m_pIdentifier[index];
}

void Node::setChildIdentifier(size_t index, id_type id)
{
	throw Tools::IllegalStateException("This should never be called. ");
}

void Node::getChildShape(id_type id, IS_SDE::IShape** out) const
{
	throw Tools::IllegalStateException("This should never be called. ");
}

void Node::getChildShape(size_t index, IShape** out)  const
{
	assert(index < m_children);

	if (isIndex() || index >= m_children)
	{
		*out = NULL;
		return;
	}

	NVDataPublish::Features::Data d(m_pDataLength[index], m_pData[index], *m_ptrMBR[index], m_pIdentifier[index], m_tree->m_condenseStrategy);
	d.getShape(out);
}

size_t Node::getLevel() const
{
	throw Tools::IllegalStateException("This should never be called. ");
}

bool Node::isIndex() const 
{ 
	return !m_bLeaf;
}

bool Node::isLeaf() const 
{ 
	return m_bLeaf;
}

RectPtr Node::getRectPtr() const 
{
	return m_tree->m_rectPool.acquire();
}

size_t Node::getIndexByteArraySize(size_t cChild)
{
	return
		CHILDREN_COUNT_SIZE		+	// m_children
		MBR_SIZE				+	// m_nodeMBR
		cChild * (
			MBR_SIZE					+	// m_ptrMBR
			EDGE_ID_SIZE				+	// m_pIdentifier
			EACH_CHILD_SIZE_SIZE			// m_pDataLength
		);
}

//
// Internal
//

Node::Node(SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree* pTree, id_type id) :
	m_identifier(id),
	m_children(0),
	m_pData(NULL),
	m_ptrMBR(NULL),
	m_pIdentifier(NULL),
	m_pDataLength(NULL),
	m_tree(pTree),
	m_totalDataLength(0),
	m_bLeaf(false)
{
	m_nodeMBR.makeInfinite();

	try
	{
		m_pDataLength = new size_t[NODE_CAPACITY + 1];
		m_pData = new byte*[NODE_CAPACITY + 1];
		m_ptrMBR = new RectPtr[NODE_CAPACITY + 1];
		m_pIdentifier = new id_type[NODE_CAPACITY + 1];
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

Node::~Node()
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

void Node::insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id)
{
	m_pDataLength[m_children] = dataLength;
	m_pData[m_children] = pData;
	m_ptrMBR[m_children] = getRectPtr();
	*(m_ptrMBR[m_children]) = mbr;
	m_pIdentifier[m_children] = id;

	m_totalDataLength += dataLength;
	++m_children;

	m_nodeMBR.combineRect(mbr);
}

void Node::readMBRFromByte(Rect& r, const byte*& ptr)
{
	int low[2], high[2];
	memcpy(low, ptr, MBR_SIZE >> 1);
	ptr += MBR_SIZE >> 1;
	memcpy(high, ptr, MBR_SIZE >> 1);
	ptr += MBR_SIZE >> 1;

	r.m_pLow[0] = low[0];
	r.m_pLow[1] = low[1];
	r.m_pHigh[0] = high[0];
	r.m_pHigh[1] = high[1];
}

void Node::writeMBRToByte(const Rect& r, byte*& ptr)
{
	int low[2], high[2];
	low[0] = static_cast<int>(r.m_pLow[0] + 0.5);
	low[1] = static_cast<int>(r.m_pLow[1] + 0.5);
	high[0] = static_cast<int>(r.m_pHigh[0] + 0.5);
	high[1] = static_cast<int>(r.m_pHigh[1] + 0.5);

	memcpy(ptr, low, MBR_SIZE >> 1);
	ptr += MBR_SIZE >> 1;
	memcpy(ptr, high, MBR_SIZE >> 1);
	ptr += MBR_SIZE >> 1;
}