#ifndef _IS_SDE_STORAGEMANAGER_STATIC_DISKSTORAGEMANAGER_H_
#define _IS_SDE_STORAGEMANAGER_STATIC_DISKSTORAGEMANAGER_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace StorageManager
	{
		class StaticDiskStorageManager : public IStorageManager
		{
		public:
			StaticDiskStorageManager(Tools::PropertySet&);
			// String       Value		Description
			// ----------------------------------------------------------------
			// Overwrite	VT_BOOL		overwrite the file if it exists.
			// FileName		VT_PWCHAR	.idx and .dat extensions will be added.
			// PageSize		VT_ULONG	specify the page size.

			virtual ~StaticDiskStorageManager();

			void flush();

			virtual void loadByteArray(const id_type id, size_t& len, byte** data);
			virtual void storeByteArray(id_type& id, const size_t len, const byte* const data);
			virtual void deleteByteArray(const id_type id);

		private:
			std::fstream m_dataFile;
			std::fstream m_indexFile;
			size_t m_pageSize;
			id_type m_nextPage;
			std::queue<id_type> m_emptyPages;

			byte* m_buffer;
		}; // StaticDiskStorageManager
	}
}

#endif // _IS_SDE_STORAGEMANAGER_STATIC_DISKSTORAGEMANAGER_H_
