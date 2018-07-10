//#include "FIFOBuffer.h"
//#include "StorageManager.h"
//
//using namespace IS_SDE;
//using namespace IS_SDE::NVDataPublish::Network;
//
//FIFOBuffer::FIFOBuffer(Tools::PropertySet& ps) : 
//	m_capacity(10), 
//	m_bWriteThrough(false), 
//	m_pStorageManager(new StorageManager(ps)), 
//	m_hits(0)
//{
//	Tools::Variant var = ps.getProperty("Capacity");
//	if (var.m_varType != Tools::VT_EMPTY)
//	{
//		if (var.m_varType != Tools::VT_LONG) 
//			throw Tools::IllegalArgumentException(
//			"Property Capacity must be Tools::VT_LONG"
//			);
//		m_capacity = var.m_val.ulVal;
//	}
//
//	var = ps.getProperty("WriteThrough");
//	if (var.m_varType != Tools::VT_EMPTY)
//	{
//		if (var.m_varType != Tools::VT_BOOL) 
//			throw Tools::IllegalArgumentException(
//			"Property WriteThrough must be Tools::VT_BOOL"
//			);
//		m_bWriteThrough = var.m_val.blVal;
//	}
//
//	var = ps.getProperty("PageSize");
//	if (var.m_varType != Tools::VT_EMPTY)
//	{
//		if (var.m_varType != Tools::VT_LONG)
//			throw Tools::IllegalArgumentException(
//			"Property PageSize must be Tools::VT_LONG"
//			);
//		m_pageSize = var.m_val.ulVal;
//	}
//}
//
//FIFOBuffer::~FIFOBuffer()
//{
//	assert(m_buffer.size() == m_queue.size());
//	for (std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
//	{
//		Entry* e = (*it).second;
//		id_type id = (*it).first;
//		if (e->m_bDirty) 
//			m_pStorageManager->storeByteArray(id, m_pageSize, e->m_pData);
//		delete e;
//	}
//	delete m_pStorageManager;
//}
//
////
//// IStorageManager interface
////
//
//void FIFOBuffer::loadByteArray(const id_type id, size_t& len, byte** data)
//{
//	std::map<id_type, Entry*>::iterator it = m_buffer.find(id);
//
//	if (it != m_buffer.end())
//	{
//		++m_hits;
//		Entry* e = (*it).second;
//		len = m_pageSize;
//		*data = new byte[len];
//		memcpy(*data, e->m_pData, len);
//	}
//	else
//	{
//		m_pStorageManager->loadByteArray(id, len, data);
//		Entry* e = new Entry((const byte *) *data, len);
//		addEntry(id, e);
//	}
//}
//
//void FIFOBuffer::storeByteArray(id_type& id, const size_t len, const byte* const data)
//{
//	assert(len == m_pageSize);
//
//	if (m_bWriteThrough)
//		m_pStorageManager->storeByteArray(id, len, data);
//
//	Entry* e = new Entry(data, len);
//	if (m_bWriteThrough == false) 
//		e->m_bDirty = true;
//
//	std::map<id_type, Entry*>::iterator it = m_buffer.find(id);
//	if (it != m_buffer.end())
//	{
//		delete (*it).second;
//		(*it).second = e;
//		if (m_bWriteThrough == false) 
//			++m_hits;
//	}
//	else
//		addEntry(id, e);
//}
//
//void FIFOBuffer::deleteByteArray(const id_type id)
//{
//	throw Tools::IllegalStateException("Should never be called. ");
//}
//
//void FIFOBuffer::clear()
//{
//	for (std::map<id_type, Entry*>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
//	{
//		if ((*it).second->m_bDirty)
//		{
//			id_type id = (*it).first;
//			m_pStorageManager->storeByteArray(id, m_pageSize, (const byte *) ((*it).second)->m_pData);
//		}
//
//		delete (*it).second;
//	}
//
//	m_buffer.clear();
//	while (!m_queue.empty())
//		m_queue.pop();
//	m_hits = 0;
//}
//
//size_t FIFOBuffer::getHits()
//{
//	return m_hits;
//}
//
//void FIFOBuffer::addEntry(id_type id, Entry* e)
//{
//	assert(m_buffer.size() <= m_capacity);
//
//	if (m_buffer.size() == m_capacity) 
//		removeEntry();
//	assert(m_buffer.find(id) == m_buffer.end());
//	m_buffer.insert(std::pair<id_type, Entry*>(id, e));
//	m_queue.push(id);
//}
//
//void FIFOBuffer::removeEntry()
//{
//	if (m_buffer.size() == 0) 
//		return;
//
//	id_type entry = m_queue.back();
//	std::map<id_type, Entry*>::iterator it = m_buffer.find(entry);
//	assert(it != m_buffer.end());
//
//	if ((*it).second->m_bDirty)
//	{
//		id_type id = (*it).first;
//		m_pStorageManager->storeByteArray(id, m_pageSize, (const byte *)((*it).second)->m_pData);
//	}
//
//	delete (*it).second;
//	m_buffer.erase(it);
//	m_queue.pop();
//}