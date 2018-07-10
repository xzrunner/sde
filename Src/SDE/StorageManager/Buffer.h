#ifndef _IS_SDE_STORAGEMANAGER_BUFFER_H_
#define _IS_SDE_STORAGEMANAGER_BUFFER_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class Buffer : public IBuffer
		{
		public:
			Buffer(IStorageManager& sm, Tools::PropertySet& ps);
				// String       Value		Description
				// ----------------------------------------------------------------
				// Capacity		VT_ULONG	Buffer maximum capacity.
				// WriteThrough	VT_BOOL		Enable or disable write through policy.

			virtual ~Buffer();

			//
			// IStorageManager interface
			//
			virtual void loadByteArray(const id_type id, size_t& len, byte** data);
			virtual void storeByteArray(id_type& id, const size_t len, const byte* const data);
			virtual void deleteByteArray(const id_type id);

			//virtual void getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const;

			//
			// IBuffer interface
			//
			virtual void clear();
			virtual size_t getHits();

		protected:
			class Entry
			{
			public:
				Entry(size_t l, const byte* const d) : m_pData(0), m_length(l), m_bDirty(false)
				{
					m_pData = new byte[m_length];
					memcpy(m_pData, d, m_length);
				}

				~Entry() { delete[] m_pData; }

				byte* m_pData;
				size_t m_length;
				bool m_bDirty;
			}; // Entry

			virtual void addEntry(id_type id, Entry* pEntry) = 0;
			virtual void removeEntry() = 0;

			size_t m_capacity;
			bool m_bWriteThrough;
			IStorageManager* m_pStorageManager;
			std::map<id_type, Entry*> m_buffer;
			size_t m_hits;

		};	// Buffer
	}
}

#endif // _IS_SDE_STORAGEMANAGER_BUFFER_H_