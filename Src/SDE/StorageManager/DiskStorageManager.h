#ifndef _IS_SDE_STORAGEMANAGER_DISKSTORAGEMANAGER_H_
#define _IS_SDE_STORAGEMANAGER_DISKSTORAGEMANAGER_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class DiskStorageManager : public IStorageManager
		{
		public:
			DiskStorageManager(Tools::PropertySet&);
				// String       Value		Description
				// ----------------------------------------------------------------
				// Overwrite	VT_BOOL		overwrite the file if it exists.
				// FileName		VT_PWCHAR	.idx and .dat extensions will be added.
				// PageSize		VT_ULONG	specify the page size.

			virtual ~DiskStorageManager();

			void flush();

			virtual void loadByteArray(const id_type id, size_t& len, byte** data);
			virtual void storeByteArray(id_type& id, const size_t len, const byte* const data);
			virtual void deleteByteArray(const id_type id);

			//virtual void getNodePageInfo(id_type id, size_t& length, size_t& pageSize) const;

		private:
			class Entry
			{
			public:
				size_t m_length;
				std::vector<id_type> m_pages;
			};

			std::fstream m_dataFile;
			std::fstream m_indexFile;
			size_t m_pageSize;
			id_type m_nextPage;
			std::priority_queue<id_type, std::vector<id_type>, std::greater<id_type> > m_emptyPages;
			std::map<id_type, Entry*> m_pageIndex;

			byte* m_buffer;
		}; // DiskStorageManager
	}
}

#endif // _IS_SDE_STORAGEMANAGER_DISKSTORAGEMANAGER_H_