#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_PUBLISH_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_PUBLISH_H_
#include "../Base.h"
#include <windows.h>

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			class PartitionMap;

			class Publish
			{
			public:
				Publish(Tools::PropertySet& ps);
				~Publish();

				void createMultiLayersNWData(HWND hwnd);

				static IStorageManager* loadNWStorageManager(const std::wstring& path) {
					return StorageManager::loadStaticDiskStorageManager(path);
				}

			public:
				static const std::wstring	FILE_NAME;

				static const size_t HEADER_PAGE			= 0;
				static const size_t BEGIN_INDEX_PAGE	= 1;

			private:
				void storeHeader(size_t indexPageCount, size_t tablePageStart, id_type& nextPage);
				void storeQuadtreeIndex(id_type& nextPage);
				void createAndStoreMultiAL(HWND hwnd, id_type& nextPage);
				void storeComeInNodesLookUpTable(id_type& nextPage);

			private:
				size_t m_pageSize;

				IStorageManager*			m_singleFile;
				StorageManager::IBuffer*	m_singleBuffer;

				IStorageManager*			m_multiFile;
				StorageManager::IBuffer*	m_multiBuffer;

				PartitionMap*				m_partition;

			}; // Publish
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_PUBLISH_H_