#include "Buffer.h"

using namespace IS_SDE;
using namespace IS_SDE::StorageManager;

Buffer::Buffer(IStorageManager& sm, Tools::PropertySet& ps) :
	m_capacity(10),
	m_bWriteThrough(false),
	m_pStorageManager(&sm),
	m_hits(0)
{
	Tools::Variant var = ps.getProperty("Capacity");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_ULONG) 
			throw Tools::IllegalArgumentException("Property Capacity must be Tools::VT_ULONG");
		m_capacity = var.m_val.ulVal;
	}

	var = ps.getProperty("WriteThrough");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL) 
			throw Tools::IllegalArgumentException("Property WriteThrough must be Tools::VT_BOOL");
		m_bWriteThrough = var.m_val.blVal;
	}
}

Buffer::~Buffer()
{
	for (std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
	{
		Entry* e = (*it).second;
		id_type id = (*it).first;
		if (e->m_bDirty) 
			m_pStorageManager->storeByteArray(id, e->m_length, e->m_pData);
		delete e;
	}
}

//
// IStorageManager interface
//

void Buffer::loadByteArray(const id_type id, size_t& len, byte** data)
{
	std::map<id_type, Entry*>::iterator it = m_buffer.find(id);

	if (it != m_buffer.end())
	{
		++m_hits;
		Entry* e = (*it).second;
		len = e->m_length;
		*data = new byte[len];
		memcpy(*data, e->m_pData, len);
	}
	else
	{
		m_pStorageManager->loadByteArray(id, len, data);
		Entry* e = new Entry(len, (const byte *) *data);
		addEntry(id, e);
	}
}

void Buffer::storeByteArray(id_type& id, const size_t len, const byte* const data)
{
	if (id == NewPage)
	{
		m_pStorageManager->storeByteArray(id, len, data);
		assert(m_buffer.find(id) == m_buffer.end());
		Entry* e = new Entry(len, data);
		addEntry(id, e);
	}
	else
	{
		if (m_bWriteThrough)
			m_pStorageManager->storeByteArray(id, len, data);

		Entry* e = new Entry(len, data);
		if (m_bWriteThrough == false) 
			e->m_bDirty = true;

		std::map<id_type, Entry*>::iterator it = m_buffer.find(id);
		if (it != m_buffer.end())
		{
			delete (*it).second;
			(*it).second = e;
			if (m_bWriteThrough == false) 
				++m_hits;
		}
		else
			addEntry(id, e);
	}
}

void Buffer::deleteByteArray(const id_type id)
{
	std::map<id_type, Entry*>::iterator it = m_buffer.find(id);
	if (it != m_buffer.end())
	{
		delete (*it).second;
		m_buffer.erase(it);
	}

	m_pStorageManager->deleteByteArray(id);
}

//void Buffer::getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const
//{
//	m_pStorageManager->getNodePageInfo(id, length, pageSize);
//}

//
// IBuffer interface
//

void Buffer::clear()
{
	for (std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
	{
		if ((*it).second->m_bDirty)
		{
			id_type id = (*it).first;
			m_pStorageManager->storeByteArray(id, ((*it).second)->m_length, (const byte *) ((*it).second)->m_pData);
		}

		delete (*it).second;
	}

	m_buffer.clear();
	m_hits = 0;
}

size_t Buffer::getHits()
{
	return m_hits;
}

