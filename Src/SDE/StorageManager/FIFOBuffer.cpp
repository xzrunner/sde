#include "FIFOBuffer.h"

using namespace IS_SDE;
using namespace IS_SDE::StorageManager;

IBuffer* IS_SDE::StorageManager::returnFIFOBuffer(IStorageManager& sm, Tools::PropertySet& ps)
{
	IBuffer* b = new FIFOBuffer(sm, ps);
	return b;
}

IBuffer* IS_SDE::StorageManager::createNewFIFOBuffer(IStorageManager& sm, size_t capacity, bool bWriteThrough)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = capacity;
	ps.setProperty("Capacity", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = bWriteThrough;
	ps.setProperty("WriteThrough", var);

	return returnFIFOBuffer(sm, ps);
}

FIFOBuffer::FIFOBuffer(IStorageManager& sm, Tools::PropertySet& ps) : Buffer(sm, ps)
{
}

FIFOBuffer::~FIFOBuffer()
{
}

void FIFOBuffer::addEntry(id_type id, Entry* e)
{
	assert(m_buffer.size() <= m_capacity);

	if (m_buffer.size() == m_capacity) 
		removeEntry();
	assert(m_buffer.find(id) == m_buffer.end());
	m_buffer.insert(std::pair<id_type, Entry*>(id, e));
	m_queue.push(id);
}

void FIFOBuffer::removeEntry()
{
	if (m_buffer.size() == 0) 
		return;

	id_type entry = m_queue.back();
	std::map<id_type, Entry*>::iterator it = m_buffer.find(entry);
	assert(it != m_buffer.end());

	if ((*it).second->m_bDirty)
	{
		id_type id = (*it).first;
		m_pStorageManager->storeByteArray(id, ((*it).second)->m_length, (const byte *)((*it).second)->m_pData);
	}

	delete (*it).second;
	m_buffer.erase(it);
	m_queue.pop();
}