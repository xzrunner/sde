#ifndef _IS_SDE_STORAGEMANAGER_MEMORYSTORAGEMANAGER_H_
#define _IS_SDE_STORAGEMANAGER_MEMORYSTORAGEMANAGER_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class MemoryStorageManager : public IStorageManager
		{
		public:
			MemoryStorageManager(Tools::PropertySet&);

			virtual ~MemoryStorageManager();

			virtual void loadByteArray(const id_type id, size_t& len, byte** data);
			virtual void storeByteArray(id_type& id, const size_t len, const byte* const data);
			virtual void deleteByteArray(const id_type id);

			//virtual void getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const;

		private:
			class Entry
			{
			public:
				byte* m_pData;
				size_t m_length;

				Entry(size_t l, const byte* const d) : m_pData(0), m_length(l)
				{
					m_pData = new byte[m_length];
					memcpy(m_pData, d, m_length);
				}

				~Entry() { delete[] m_pData; }
			}; // Entry

			std::vector<Entry*> m_buffer;
			std::stack<id_type> m_emptyPages;
		}; // MemoryStorageManager
	}
}

#endif // _IS_SDE_STORAGEMANAGER_MEMORYSTORAGEMANAGER_H_