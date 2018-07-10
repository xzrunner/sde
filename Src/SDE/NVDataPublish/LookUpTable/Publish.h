#ifndef _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_PUBLISH_H_
#define _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_PUBLISH_H_
#include "../Base.h"
#include <windows.h>

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace LookUpTable
		{
			class Publish
			{
			public:
				Publish(Tools::PropertySet& ps);
				~Publish();

				void createRegularTable(HWND hwnd);
				void createQuadTable(HWND hwnd);
				
				static IStorageManager* loadTableStorageManager(const std::wstring& path) {
					return StorageManager::loadStaticDiskStorageManager(path);
				}

			public:
				static const std::wstring	FILE_NAME;
				static const size_t			PAGE_SIZE = 4096;

			private:
				IStorageManager*			m_nwFile;
				StorageManager::IBuffer*	m_nwBuffer;

				IStorageManager*			m_tableFile;
				StorageManager::IBuffer*	m_tableBuffer;

				ILookUp* m_lookUp;

				size_t m_capacity;

			}; // Publish
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_PUBLISH_H_
