#include "../Tools/rand48.h"
#include "RandomEvictionsBuffer.h"

using namespace IS_SDE;
using namespace IS_SDE::StorageManager;

IBuffer* IS_SDE::StorageManager::returnRandomEvictionsBuffer(IStorageManager& sm, Tools::PropertySet& ps)
{
	IBuffer* b = new RandomEvictionsBuffer(sm, ps);
	return b;
}

IBuffer* IS_SDE::StorageManager::createNewRandomEvictionsBuffer(IStorageManager& sm, size_t capacity, bool bWriteThrough)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = capacity;
	ps.setProperty("Capacity", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = bWriteThrough;
	ps.setProperty("WriteThrough", var);

	return returnRandomEvictionsBuffer(sm, ps);
}

RandomEvictionsBuffer::RandomEvictionsBuffer(IStorageManager& sm, Tools::PropertySet& ps) : Buffer(sm, ps)
{
	srand48(time(NULL));
}

RandomEvictionsBuffer::~RandomEvictionsBuffer()
{
}

void RandomEvictionsBuffer::addEntry(id_type id, Entry* e)
{
	assert(m_buffer.size() <= m_capacity);

	if (m_buffer.size() == m_capacity) 
		removeEntry();
	assert(m_buffer.find(id) == m_buffer.end());
	m_buffer.insert(std::pair<id_type, Entry*>(id, e));
}

void RandomEvictionsBuffer::removeEntry()
{
	if (m_buffer.size() == 0) 
		return;

	double random;

	random = drand48();

	size_t entry = static_cast<size_t>(floor(((double) m_buffer.size()) * random));

	std::map<id_type, Entry*>::iterator it = m_buffer.begin();
	for(size_t cIndex = 0; cIndex < entry; ++cIndex) 
		++it;

	if ((*it).second->m_bDirty)
	{
		id_type id = (*it).first;
		m_pStorageManager->storeByteArray(id, ((*it).second)->m_length, (const byte *)((*it).second)->m_pData);
	}

	delete (*it).second;
	m_buffer.erase(it);
}
