#include "MemoryStorageManager.h"

using namespace IS_SDE::StorageManager;
typedef IS_SDE::id_type id_type;

IS_SDE::IStorageManager* IS_SDE::StorageManager::returnMemoryStorageManager(Tools::PropertySet& ps)
{
	IStorageManager* sm = new MemoryStorageManager(ps);
	return sm;
}

IS_SDE::IStorageManager* IS_SDE::StorageManager::createNewMemoryStorageManager()
{
	Tools::PropertySet ps;
	return returnMemoryStorageManager(ps);
}

MemoryStorageManager::MemoryStorageManager(Tools::PropertySet& ps)
{
}

MemoryStorageManager::~MemoryStorageManager()
{
	for_each(m_buffer.begin(), m_buffer.end(), Tools::DeletePointerFunctor<Entry>());
}

void MemoryStorageManager::loadByteArray(const id_type id, size_t& len, byte** data)
{
	Entry* e;
	try
	{
		e = m_buffer.at(id);
		if (e == 0) 
			throw Tools::InvalidPageException(id);
	}
	catch (std::out_of_range)
	{
		throw Tools::InvalidPageException(id);
	}

	len = e->m_length;
	*data = new byte[len];

	memcpy(*data, e->m_pData, len);
}

void MemoryStorageManager::storeByteArray(id_type& id, const size_t len, const byte* const data)
{
	if (id == NewPage)
	{
		Entry* e = new Entry(len, data);

		if (m_emptyPages.empty())
		{
			m_buffer.push_back(e);
			id = m_buffer.size() - 1;
		}
		else
		{
			id = m_emptyPages.top(); m_emptyPages.pop();
			m_buffer[id] = e;
		}
	}
	else
	{
		Entry* e_old;
		try
		{
			e_old = m_buffer.at(id);
			if (e_old == 0) 
				throw Tools::InvalidPageException(id);
		}
		catch (std::out_of_range)
		{
			throw Tools::InvalidPageException(id);
		}

		Entry* e = new Entry(len, data);

		delete e_old;
		m_buffer[id] = e;
	}
}

void MemoryStorageManager::deleteByteArray(const id_type id)
{
	Entry* e;
	try
	{
		e = m_buffer.at(id);
		if (e == 0) throw Tools::InvalidPageException(id);
	}
	catch (std::out_of_range)
	{
		throw Tools::InvalidPageException(id);
	}

	m_buffer[id] = 0;
	m_emptyPages.push(id);

	delete e;
}

//void MemoryStorageManager::getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const
//{
//	throw Tools::NotSupportedException(
//		"MemoryStorageManager::getNodePageInfo: not support. "
//		);
//}