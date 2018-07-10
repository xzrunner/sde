#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_CONDENSE_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_CONDENSE_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace Render
	{
		class MapLayer;
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class StaticRTree;
			}
		}
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class CondenseData
			{
			public:
				CondenseData(
					Render::MapLayer* src, 
					size_t layerID, 
					Tools::PropertySet& ps
					);
				~CondenseData();

				void bulkLoadData(size_t layerID);

				size_t getLayerDataSizeAfterCondense() const;

				STATIC_LAYER* getReconstructedLayer() { return m_dest; }

				std::wstring getLayerName() const { 
					return m_filePath.substr(m_filePath.find_last_of(L'\\') + 1, m_filePath.find_last_of(L'.') - m_filePath.find_last_of(L'\\'));
				}

			private:
				static size_t getNodeByteArraySize(size_t cChild);

			private:
				class CountCondenseSizeQueryStrategy : public IQueryStrategy
				{
				public:
					CountCondenseSizeQueryStrategy(NVDataPublish::ICondenseStrategy* condenseStrategy);

				public:
					void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext);
					bool shouldStoreEntry() { return false; }

				private:
					NVDataPublish::ICondenseStrategy* m_condenseStrategy;
					std::queue<id_type> m_ids;

					size_t m_size;

					friend class CondenseData;

				}; // CountCondenseSizeQueryStrategy

			private:
				ISpatialIndex* m_src;

				STATIC_LAYER* m_dest;
				IStorageManager* m_file;
				// m_dest is directly loaded from m_src, without using the ISpatialIndex struct. 
				// todo: use the struct of src to accelerate the process. 

				std::wstring m_filePath;

				size_t m_layerID;
				// use to specify the road id.

				size_t m_pageSize;
				// Bulk Loader use it.

				ICrt* m_trans;

				ICondenseStrategy* m_strategy;

				friend class LayerBulkLoader;

			}; // CondenseData
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_CONDENSE_H_