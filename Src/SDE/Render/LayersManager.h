#ifndef _IS_SDE_RENDER_LAYERS_MANAGER_H_
#define _IS_SDE_RENDER_LAYERS_MANAGER_H_
#include "../Tools/Tools.h"

namespace IS_SDE
{
	class Rect;

	namespace EDIT
	{
		class EditTask;
		class EditController;
	}

	namespace Render
	{
		class MapLayer;
		class MapView;
		class LayersManager
		{
		public:
			LayersManager();
			~LayersManager();

			bool loadLayer(const std::wstring& filePath, MapView& view);
			bool isLayerExist(const std::wstring& layerName) const;
			void flipVisible(size_t index);
			Rect* getTotDisplayScope() const;
			size_t layersSize() const { return m_layerWithVisible.size(); }

			MapLayer* getFirstVisibleMapLayer();
			void getVisibleLayers(std::vector<Render::MapLayer*>& ret);

			bool isNoneVisible() const { return m_countVisibleLayer == 0; }

			void storeDisplayRegion(const Rect& scope);
			
		private:
			bool loadLayer(const std::wstring& path, const std::wstring& name, MapView& view);

		private:
			std::vector<std::pair<MapLayer*, bool> > m_layerWithVisible;
			std::set<std::wstring> m_layersName;

			size_t m_countVisibleLayer;

		}; // LayersManager
	}
}

#endif // _IS_SDE_RENDER_LAYERS_MANAGER_H_