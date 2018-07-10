#include "SelectiveBySP.h"
#include "../../../NVDataPublish/Network/Publish.h"
#include "../../../NVDataPublish/SPClassification/Utility.h"
#include "../../../SimulateNavigate/Network/SingleLayerNW.h"
#include "../../../SimulateNavigate/SpatialIndex/StaticRTree/PointBindLine.h"
#include "../../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"
#include "../../../Render/MapLayer.h"
#include "../../../BasicType/Line.h"
#include "../../../SpatialIndex/Utility/QueryStrategy.h"
#include "../../../SpatialIndex/Utility/Visitor.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

SelectiveBySP::SelectiveBySP(const std::wstring& topoFilePath, size_t divideNum) 
	: m_nDivide(divideNum)
{
	m_file = NVDataPublish::Network::Publish::loadNWStorageManager(topoFilePath);
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, 10, false);

	std::wstring dirPath = topoFilePath.substr(0, topoFilePath.find_last_of(L'\\'));
	initLayers(m_buffer, dirPath);

	const NVDataPublish::ICondenseStrategy* cs 
		= dynamic_cast<NVDataPublish::STATIC_LAYER*>(m_indexes.at(0))->getCondenseStrategy();
	std::vector<ISpatialIndex*> indexes;
	copy(m_indexes.begin(), m_indexes.end(), back_inserter(indexes));
	m_bind = new SimulateNavigate::SpatialIndex::StaticRTree::PointBindLine(indexes, cs);

	m_network = new SimulateNavigate::Network::SingleLayerNW(m_buffer, m_bind, NULL, false);
}

SelectiveBySP::~SelectiveBySP()
{
	delete m_network;
	delete m_bind;
	delete m_buffer;
	delete m_file;

	for_each(m_layers.begin(), m_layers.end(), Tools::DeletePointerFunctor<Render::MapLayer>());
}

void SelectiveBySP::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	computeSPRoutes();

	dest.clear();
	dest.reserve(m_destIDs.size());
	for (size_t i = 0; i < m_destIDs.size(); ++i)
	{
		Line* l;
		NVDataPublish::SPC::Utility::getLineByAddress(&m_indexes, m_destIDs[i], &l);
		dest.push_back(l);
	}
}

void SelectiveBySP::getBorderIntersectShapes(std::vector<IShape*>& shapes) const
{
	Rect totScope;
	for (size_t i = 0; i < m_indexes.size(); ++i)
	{
		SpatialIndex::LayerRegionQueryStrategy qs;
		m_indexes[i]->queryStrategy(qs);
		totScope.combineRect(qs.m_indexedSpace);
	}
	double xEdgeLen = (totScope.m_pHigh[0] - totScope.m_pLow[0]) / m_nDivide,
		yEdgeLen = (totScope.m_pHigh[1] - totScope.m_pLow[1]) / m_nDivide;

	std::vector<IShape*> borders;
	for (size_t i = 0; i < m_nDivide; ++i)
	{
		std::vector<MapPos2D> linePos;
		linePos.push_back(MapPos2D(totScope.m_pLow[0], totScope.m_pLow[1] + yEdgeLen * i));
		linePos.push_back(MapPos2D(totScope.m_pHigh[0], totScope.m_pLow[1] + yEdgeLen * i));
		borders.push_back(new Line(linePos));
	}
	for (size_t i = 0; i < m_nDivide; ++i)
	{
		std::vector<MapPos2D> linePos;
		linePos.push_back(MapPos2D(totScope.m_pLow[0] + xEdgeLen * i, totScope.m_pLow[1]));
		linePos.push_back(MapPos2D(totScope.m_pLow[0] + xEdgeLen * i, totScope.m_pHigh[1]));
		borders.push_back(new Line(linePos));
	}

	for (size_t i = 0; i < borders.size(); ++i)
	{
		std::vector<IShape*> tmp;
		SpatialIndex::FetchDataVisitor vis(tmp);
		for (size_t j = 0; j < m_indexes.size(); ++j)
			m_indexes[j]->intersectsWithQuery(*borders[i], vis, false);
		copy(tmp.begin(), tmp.end(), back_inserter(shapes));
	}

	shapes.erase(unique(shapes.begin(), shapes.end()), shapes.end());

	for_each(borders.begin(), borders.end(), Tools::DeletePointerFunctor<IShape>());
}

void SelectiveBySP::initLayers(IStorageManager* sm, const std::wstring& path)
{
	std::vector<std::wstring> layerNames;
	NVDataPublish::Network::Publish::fetchLayersName(sm, layerNames);
	size_t layerNum = layerNames.size();
	assert(layerNum);

	m_layers.clear(); m_indexes.clear();
	m_layers.reserve(layerNum);	m_indexes.reserve(layerNum);
	for (size_t i = 0; i < layerNum; ++i)
	{
		Render::MapLayer* ml = new Render::MapLayer(path + L"\\" + layerNames[i], layerNames[i]);
		m_layers.push_back(ml);
		m_indexes.push_back(dynamic_cast<NVDataPublish::STATIC_LAYER*>(ml->getSpatialIndex()));
	}
}

void SelectiveBySP::computeSPRoutes()
{
	size_t posNumRow = m_nDivide * 2 + 1;

	m_calBuffer.clear();
	m_calBuffer.resize(posNumRow * posNumRow);

	Rect totScope;
	for (size_t i = 0; i < m_indexes.size(); ++i)
	{
		SpatialIndex::LayerRegionQueryStrategy qs;
		m_indexes[i]->queryStrategy(qs);
		totScope.combineRect(qs.m_indexedSpace);
	}
	double xEdgeLen = (totScope.m_pHigh[0] - totScope.m_pLow[0]) / (posNumRow - 1),
		yEdgeLen = (totScope.m_pHigh[1] - totScope.m_pLow[1]) / (posNumRow - 1);

	for (size_t i = 0; i < posNumRow; ++i)
		for (size_t j = 0; j < posNumRow; ++j)
		{
			MapPos2D center(totScope.m_pLow[0] + xEdgeLen * j, totScope.m_pLow[1] + yEdgeLen * i);
			Rect scope(MapPos2D(center.d0 - xEdgeLen, center.d1 - yEdgeLen), MapPos2D(center.d0 + xEdgeLen, center.d1 + yEdgeLen));
			std::set<uint64_t> ids;
			NVDataPublish::SPC::Utility::getCondensedRoadIDsByExpandToOthers(m_indexes, scope, m_network, ids);
			copy(ids.begin(), ids.end(), back_inserter(m_calBuffer[i * posNumRow + j]));
		}

	for (size_t i = 0; i < m_nDivide; ++i)
		for (size_t j = 0; j < m_nDivide; ++j)
		{
			size_t centerID = (i * 2 + 1) * posNumRow + (j * 2 + 1);

			calSetsIntersection(centerID, centerID - 1, centerID - posNumRow, centerID - 1 - posNumRow);
			calSetsIntersection(centerID, centerID - 1, centerID + posNumRow, centerID - 1 + posNumRow);
			calSetsIntersection(centerID, centerID + 1, centerID - posNumRow, centerID + 1 - posNumRow);
			calSetsIntersection(centerID, centerID + 1, centerID + posNumRow, centerID + 1 + posNumRow);
		}
}

void SelectiveBySP::calSetsIntersection(size_t g0, size_t g1, size_t g2, size_t g3)
{
	std::vector<uint64_t> tmpIDs;
	copy(m_calBuffer[g0].begin(), m_calBuffer[g0].end(), back_inserter(tmpIDs));
	copy(m_calBuffer[g1].begin(), m_calBuffer[g1].end(), back_inserter(tmpIDs));
	copy(m_calBuffer[g2].begin(), m_calBuffer[g2].end(), back_inserter(tmpIDs));
	copy(m_calBuffer[g3].begin(), m_calBuffer[g3].end(), back_inserter(tmpIDs));

	sort(tmpIDs.begin(), tmpIDs.end());
	if (!tmpIDs.empty())
	{
		uint64_t currID = tmpIDs.front();
		size_t count = 1;
		for (size_t i = 1; i < tmpIDs.size(); ++i)
		{
			if (tmpIDs[i] != currID)
			{
				if (count == 4)
					m_destIDs.push_back(currID);
				currID = tmpIDs[i];
				count = 1;
			}
			else
				++count;
		}
	}
}
