#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_PUBLISH_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_PUBLISH_H_
#include "../Base.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace Render
	{
		class MapLayer;
	}

	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class NVRTree;
		}
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class Publish;
		}

		namespace Network
		{
			class PartitionMap;
			class AdjacencyList;

			class Publish
			{
			public:
				Publish(const Features::Publish& fp, Tools::PropertySet& ps);
				Publish(const std::vector<Render::MapLayer*>& layers, Tools::PropertySet& ps);
				~Publish();

				void createNetworkData();

				size_t queryTopoNodeID(const MapPos2D& p) const;

				void readPageData(size_t pageID, byte** data) const;
				void writePageData(size_t pageID, const byte* const data);

				static IStorageManager* loadNWStorageManager(const std::wstring& path) {
					return StorageManager::loadStaticDiskStorageManager(path);
				}
				static void fetchLayersName(IStorageManager* sm, std::vector<std::wstring>& names);

			private:
				void init(Tools::PropertySet& ps);
				void createTopoData(size_t& nextPage);
				void createLeafData(size_t& nextPage);
				void createIndexData(size_t& nextPage);
				void createHeader(size_t firstLeafPage, size_t firstIndexPage, bool rootIsLeaf);

			public:
				static const size_t LEAF_COUNT_SIZE	= 1;

				static const size_t HEADER_PAGE		= 0;
				static const size_t BEGIN_AL_PAGE	= 1;

				static const std::wstring FILE_NAME;

			private:
				std::vector<STATIC_LAYER*> m_layers;
				std::vector<std::wstring> m_layersName;

				// <1> Header
				// <2> AdjacencyList
				// <3> Leaf
				// <4> Index
				IStorageManager*			m_file;
				StorageManager::IBuffer*	m_buffer;
				size_t m_pageSize;

				PartitionMap* m_partitionMap;

				friend class AdjacencyList;
			};
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_PUBLISH_H_

