#ifndef _IS_SDE_RENDER_MAP_LAYER_H_
#define _IS_SDE_RENDER_MAP_LAYER_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Render
	{
		class MapView;
		class MapLayer
		{
		public:
			MapLayer(const std::wstring& filePath, const std::wstring layerName);
			MapLayer(const std::wstring& filePath, const std::wstring layerName, MapView& view);
			~MapLayer();

			IS_SDE::ISpatialIndex* getSpatialIndex();

			const std::wstring& getLayerName() const { return m_layerName; }
			std::wstring getFilePath() const { return m_filePath; }

		private:
			void loadSpatialIndex();
			void loadRTree();
			void loadNVRTree();
			void loadQuadTree();
			void loadStaticRTree();

		private:
			IStorageManager*			m_file;
			StorageManager::IBuffer*	m_buffer;
			ISpatialIndex*				m_spatialIndex;

			std::wstring				m_layerName;
			std::wstring				m_filePath;

		}; // MapLayer
	}
}

#endif // _IS_SDE_RENDER_MAP_LAYER_H_