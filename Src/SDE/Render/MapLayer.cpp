#include "MapLayer.h"
#include "../Render/MapView.h"
#include "../SpatialIndex/RTree/RTree.h"
#include "../SpatialIndex/NVRTree/NVRTree.h"
#include "../SpatialIndex/QuadTree/QuadTree.h"
#include "../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"
#include "../Tools/StringTools.h"

using namespace IS_SDE;
using namespace IS_SDE::Render;

MapLayer::MapLayer(const std::wstring& filePath, const std::wstring layerName)
	: m_filePath(filePath), m_layerName(layerName)
{
	loadSpatialIndex();
}

MapLayer::MapLayer(const std::wstring& filePath, const std::wstring layerName, MapView& view)
	: m_filePath(filePath), m_layerName(layerName)
{
	loadSpatialIndex();
	view.setViewScope(MapScope(m_spatialIndex->getDisplayRegion()));
}

MapLayer::~MapLayer()
{
	delete m_spatialIndex;
	delete m_buffer;
	delete m_file;
}

IS_SDE::ISpatialIndex* MapLayer::getSpatialIndex()
{
	return m_spatialIndex;
}

void MapLayer::loadSpatialIndex()
{
	std::wstring type = Tools::StringTools::substrFromBack(m_filePath, '_', '.');
	if (type == L"Quad")
		loadQuadTree();
	else if (type == L"R")
		loadRTree();
	else if (type == L"NVR")
		loadNVRTree();
	else if (type == L"STATIC")
		loadStaticRTree();
	else
		throw Tools::IllegalArgumentException(
		"MapLayer::loadSpatialIndex: Error SpatialIndex Type. "
		);
}

void MapLayer::loadRTree()
{
	std::wstring idxPath = m_filePath.substr(0, m_filePath.find_last_of('.'));
	m_file = StorageManager::loadDiskStorageManager(idxPath);
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, 10, false);
	m_spatialIndex = SpatialIndex::RTree::loadRTree(*m_buffer, 1);
}

void MapLayer::loadNVRTree()
{
	std::wstring idxPath = m_filePath.substr(0, m_filePath.find_last_of('.'));
	m_file = StorageManager::loadDiskStorageManager(idxPath);
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, 100, false);
	m_spatialIndex = SpatialIndex::NVRTree::loadNVRTree(*m_buffer, 1);
}

void MapLayer::loadQuadTree()
{
	std::wstring idxPath = m_filePath.substr(0, m_filePath.find_last_of('.'));
	m_file = StorageManager::loadDiskStorageManager(idxPath);
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, 100, false);
	m_spatialIndex = SpatialIndex::QuadTree::loadQuadTree(*m_buffer, SpatialIndex::QuadTree::MAX_ID);
}

void MapLayer::loadStaticRTree()
{
	std::wstring idxPath = m_filePath.substr(0, m_filePath.find_last_of('.'));
	m_file = StorageManager::loadStaticDiskStorageManager(idxPath);
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, 100, false);
	m_spatialIndex = SimulateNavigate::SpatialIndex::StaticRTree::loadStaticRTree(*m_buffer, 1);
}