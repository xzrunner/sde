#include "LayersManager.h"
#include "../Tools/StringTools.h"
#include "MapLayer.h"
#include "../SpatialIndex/Utility/QueryStrategy.h"
#include "../NVDataPublish/Network/Publish.h"

using namespace IS_SDE;
using namespace IS_SDE::Render;

LayersManager::LayersManager()
	: m_countVisibleLayer(0)
{
}

LayersManager::~LayersManager()
{
	std::vector<std::pair<MapLayer*, bool> >::iterator itr = m_layerWithVisible.begin();
	for ( ; itr != m_layerWithVisible.end(); ++itr)
		delete itr->first;
}

bool LayersManager::loadLayer(const std::wstring& filePath, MapView& view)
{
	std::wstring layerName = Tools::StringTools::substrFromBack(filePath, '\\', '.');
	if (layerName == NVDataPublish::Network::Publish::FILE_NAME)
	{
		IStorageManager* sm = NVDataPublish::Network::Publish::loadNWStorageManager(
			filePath.substr(0, filePath.find_last_of('.'))
			);
		std::wstring dirPath = filePath.substr(0, filePath.find_last_of(L'\\'));

		std::vector<std::wstring> layerNames;
		NVDataPublish::Network::Publish::fetchLayersName(sm, layerNames);

		bool bOpen = false;
		for (size_t i = 0; i < layerNames.size(); ++i)
		{
			if (loadLayer(dirPath + L"\\" + layerNames[i], layerNames[i], view))
				bOpen = true;
		}
		return bOpen;
	}
	else
		return loadLayer(filePath, layerName, view);
}

bool LayersManager::isLayerExist(const std::wstring& layerName) const
{
	return m_layersName.find(layerName) != m_layersName.end();
}

void LayersManager::flipVisible(size_t index)
{
	m_layerWithVisible.at(index).second = !m_layerWithVisible.at(index).second;
	if (m_layerWithVisible.at(index).second)
		++m_countVisibleLayer;
	else
		--m_countVisibleLayer;
}

Rect* LayersManager::getTotDisplayScope() const
{
	Rect* r = new Rect;
	for (size_t i = 0; i < m_layerWithVisible.size(); ++i)
	{
		if (m_layerWithVisible.at(i).second)
		{
			ISpatialIndex* spatialIndex 
				= m_layerWithVisible.at(i).first->getSpatialIndex();

			SpatialIndex::LayerRegionQueryStrategy qs;
			spatialIndex->queryStrategy(qs);
			r->combineRect(qs.m_indexedSpace);
		}
	}

	return r;
}

MapLayer* LayersManager::getFirstVisibleMapLayer()
{
	MapLayer* ret = NULL;

	for (size_t i = 0; i < m_layerWithVisible.size(); ++i)
	{
		if (m_layerWithVisible.at(i).second)
		{
			ret = m_layerWithVisible.at(i).first;
			break;
		}
	}

	return ret;
}

void LayersManager::getVisibleLayers(std::vector<MapLayer*>& ret)
{
	std::vector<std::pair<MapLayer*, bool> >::iterator itr = m_layerWithVisible.begin();
	for (; itr != m_layerWithVisible.end(); ++itr)
	{
		if (itr->second)
			ret.push_back(itr->first);
	}
}

void LayersManager::storeDisplayRegion(const Rect& scope)
{
	std::vector<std::pair<MapLayer*, bool> >::iterator itr = m_layerWithVisible.begin();
	for (; itr != m_layerWithVisible.end(); ++itr)
	{
		if (itr->second)
		{
			itr->first->getSpatialIndex()->setDisplayRegion(scope);
		}	
	}
}

bool LayersManager::loadLayer(const std::wstring& path, const std::wstring& name, MapView& view)
{
	if (!isLayerExist(name))
	{
		m_layerWithVisible.push_back(
			std::make_pair(new MapLayer(path, name, view), true)
			);
		m_layersName.insert(name);
		++m_countVisibleLayer;
		return true;
	}
	return false;
}