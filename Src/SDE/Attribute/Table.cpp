#include "Table.h"
#include "Fields.h"
#include "Row.h"
using namespace IS_SDE;
using namespace IS_SDE::Attribute;

Node::Node(size_t capacity)
	: m_capacity(capacity)
{
}

//
// ISerializable interface
//

size_t Node::getByteArraySize() const
{
	size_t ret = 0;
	std::map<id_type, StaticRow*>::const_iterator itr = m_GOIDToRow.begin();
	for ( ; itr != m_GOIDToRow.end(); ++itr)
		ret += itr->second->getByteArraySize();
	return ret;
}

void Node::loadFromByteArray(const byte* data)
{
	//id_type id;
	//memcpy(&id, data, sizeof(id_type));
	//StaticRow* row = new StaticRow;
	//row->loadFromByteArray(data);
}

void Node::storeToByteArray(byte** data, size_t& len) const
{
	len = getByteArraySize();
	*data = new byte[len];
	byte* ptr = *data;

	std::map<id_type, StaticRow*>::const_iterator itr = m_GOIDToRow.begin();
	for ( ; itr != m_GOIDToRow.end(); ++itr)
	{
		memcpy(ptr, &itr->first, sizeof(id_type));
		ptr += sizeof(id_type);

		byte* rowData;
		size_t rowLen;
		itr->second->storeToByteArray(&rowData, rowLen);

		memcpy(ptr, rowData, rowLen);
		ptr += rowLen;
		delete[] rowData;
	}
	delete[] data;
}

bool Node::insertData(id_type id, size_t len, const byte* pData)
{
	assert(m_capacity >= len);

	return true;
}

// Table

Table::Table(size_t nodeCapacity)
	: m_nodeCapacity(nodeCapacity), m_currNode(NULL), m_lastKey(-1)
{
}

void Table::insertData(const id_type& key, size_t len, const byte* pData)
{
	if (len > m_nodeCapacity)
		throw Tools::IllegalStateException(
		"Table::insertData: The item can't load into a node. "
		);

	if (key <= m_lastKey)
		throw Tools::IllegalStateException(
		"Table::insertData: The key should sort ascending. "
		);

	if (m_currNode->m_capacity >= len)
	{
		m_currNode->insertData(key, len, pData);
	}
	else
	{
		writeNode(m_currNode);
		delete m_currNode;
		m_currNode = new Node(m_nodeCapacity);
		m_currNode->insertData(key, len, pData);
	}
}

bool Table::deleteData(const id_type& key)
{
	throw Tools::NotSupportedException("Not support. ");
}

bool Table::equalsQuery(const id_type& key, IRow& r)
{
	std::map<id_type, id_type>::iterator itr = m_GOIDToNodeID.lower_bound(key);
	if (itr == m_GOIDToNodeID.end())
		return false;

	//Node* n = readNode(itr->second);
	//n->
}

id_type Table::writeNode(Node* n)
{
	return 0;
}

Node* Table::readNode(id_type id)
{
	size_t dataLength;
	byte* buffer;

	try
	{
		m_pStorageManager->loadByteArray(id, dataLength, &buffer);
	}
	catch (Tools::InvalidPageException& e)
	{
		std::cerr << e.what() << std::endl;
		throw Tools::IllegalStateException("readNode: failed with Tools::InvalidPageException");
	}

	try
	{
		Node* n = new Node(m_nodeCapacity);
		n->loadFromByteArray(buffer);
		delete[] buffer;
		return n;
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}
}