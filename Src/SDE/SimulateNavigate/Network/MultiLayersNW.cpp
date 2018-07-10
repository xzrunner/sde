#include "MultiLayersNW.h"
#include "SingleLayerNW.h"
#include "BindPoint.h"
#include "MultiLayersSearching.h"
#include "../../NVDataPublish/MultiLayersNW/Publish.h"
#include "../../NVDataPublish/MultiLayersNW/EnterGridNodesMapping.h"
#include "../../NVDataPublish/Network/AdjacencyList.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

typedef NVDataPublish::Network::AdjacencyList							S_AL;
typedef NVDataPublish::MLNW::AdjacencyList								M_AL;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::Layer1IndexStorage	LAYER_1_INDEX;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::Layer2IndexStorage	LAYER_2_INDEX;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::Entry				ENTRY;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::SINGLE_NODE_ID		SINGLE_NODE_ID;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::MULTI_NODE_ID		MULTI_NODE_ID;
typedef NVDataPublish::MLNW::EnterGridNodesMapping::ADDRESS				ADDRESS;

MultiLayersNW::MultiLayersNW(IStorageManager* sm, const SingleLayerNW* singleNW, 
							 const SimulateNavigate::IPointBindLine* bind, 
							 const NVDataPublish::ILookUp* table, bool bPreLoadTable)
	: m_buffer(sm), m_singleNW(singleNW), m_bind(bind), m_table(table), m_bPreLoadTable(bPreLoadTable)
{
	size_t pageSize, indexPageCount, tablePageStart;
	parseHeader(pageSize, indexPageCount, tablePageStart);
	m_spatialIndex.initByParseStorage(m_buffer, pageSize, indexPageCount);
	m_comeInNodesLookUpTable.initByParseStorage(m_buffer, pageSize, tablePageStart);
}

double MultiLayersNW::getShortestDistance(size_t src, size_t dest) const
{
	MultiLayersSearching search(m_singleNW, this, NULL, false);
	return search.queryShortestDistance(src, dest);
}

double MultiLayersNW::computeShortestPath(SimulateNavigate::SingleRouteSearchingInfo& info) const
{
	BindPoint s(info.m_start.m_realPos, m_bind, m_singleNW), 
		e(info.m_end.m_realPos, m_bind, m_singleNW);
	info.m_start.m_bindPos = s.m_pos;
	info.m_end.m_bindPos = e.m_pos;

	std::vector<std::pair<NODE_ID, NEAREST_DIS> > route;

	MultiLayersSearching search(m_singleNW, this, info.m_disPro, m_table, m_bPreLoadTable);
	double dis = search.queryShortestRoute(s, e, &route, &info.m_expandNum);

	parseRouteResult(route, s, e, info);

	return dis;
}

void MultiLayersNW::computeShortestPath(SimulateNavigate::MultiRoutesSearchingInfo& info) const
{
	throw Tools::NotSupportedException(
		"MultiLayersNW::computeShortestPath(MultiRoutesSearchingInfo& info): Should not be called. "
		);
}

void MultiLayersNW::getTopoNodeConnection(size_t nodeID, std::vector<M_CONN>* conns) const
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(nodeID, &page, &offset);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	M_AL::fetchConnectionInfo(ptr, conns);

	delete[] data;
}

size_t MultiLayersNW::getTopoNodeLowID(size_t nodeID) const
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(nodeID, &page, &offset);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	size_t lowID = M_AL::fetchSingleTopoNodeID(ptr);

	delete[] data;

	return lowID;
}

size_t MultiLayersNW::getCrossGridHighID(size_t expandLowID, const MapPos2D& dest0, 
										 const MapPos2D& dest1, bool bDestSameGrid) const
{
	MapPos2D expand;
	m_singleNW->getTopoNodePos(expandLowID, &expand);
	size_t layer0 = m_spatialIndex.getHighestLayerGridBetweenPos(expand, dest0);
	if (bDestSameGrid)
	{
		if (layer0 == 0)
			return 0;
		else
			return m_comeInNodesLookUpTable.mapSingleIDAndLayerToMultiID(expandLowID, layer0);
	}
	else
	{
		size_t layer1 = m_spatialIndex.getHighestLayerGridBetweenPos(expand, dest1);
		if (layer0 == 0 && layer1 == 0)
			return 0;
		else if (layer0 == 0)
			return m_comeInNodesLookUpTable.mapSingleIDAndLayerToMultiID(expandLowID, layer1);
		else if (layer1 == 0)
			return m_comeInNodesLookUpTable.mapSingleIDAndLayerToMultiID(expandLowID, layer0);
		else
			return m_comeInNodesLookUpTable.mapSingleIDAndLayerToMultiID(expandLowID, std::min(layer0, layer1));
	}
}

//
// class MultiLayersNW::SpatialIndex
//

void MultiLayersNW::SpatialIndex::
	initByParseStorage(IStorageManager* storage, size_t pageSize, size_t indexPageCount)
{
	size_t totLen = pageSize * indexPageCount;
	byte* totData = new byte[totLen];
	byte* ptr = totData;
	for (size_t i = 0; i < indexPageCount; ++i)
	{
		byte* data;
		size_t len;
		storage->loadByteArray(NVDataPublish::MLNW::Publish::BEGIN_INDEX_PAGE + i, len, &data);
		assert(len == pageSize);
		memcpy(ptr, data, len);
		ptr += len;
		delete[] data;
	}

	ptr = totData;
	memcpy(m_scope.m_pLow, ptr, sizeof(double) * 2);
	ptr += sizeof(double) * 2;
	memcpy(m_scope.m_pHigh, ptr, sizeof(double) * 2);
	ptr += sizeof(double) * 2;

	m_root = new Grid;
	if ((*ptr & 0x80) != 0)
	{
		byte test = *ptr;
		test <<= 1;
		size_t rem = 7;

		std::stack<std::pair<Grid*, size_t> > buffer;
		buffer.push(std::make_pair(m_root, 0));
		while (!buffer.empty())
		{
			std::pair<Grid*, size_t>& item = buffer.top();
			Grid* g = new Grid;
			item.first->m_child[item.second++] = g;
			if (item.second == 4)
				buffer.pop();
			if ((test & 0x80) != 0)
				buffer.push(std::make_pair(g, 0));

			test <<= 1;
			--rem;
			if (rem == 0)
			{
				++ptr;
				test = *ptr;
				rem = 8;
			}
		}
	}

	delete[] totData;
}

size_t MultiLayersNW::SpatialIndex::
	getHighestLayerGridBetweenPos(const MapPos2D& locate, const MapPos2D& except) const
{
	size_t ret = 0;
	assert(m_scope.containsPoint(locate) && m_scope.containsPoint(except));

	Rect currScope(m_scope);
	Grid* currGrid(m_root);
	do 
	{
		if (currGrid->isLeaf())
			return 0;

		MapPos2D center(currScope.center());

		if (locate.d0 < center.d0)
		{
			if (locate.d1 < center.d1)
			{
				currScope.m_pHigh[0] = center.d0;
				currScope.m_pHigh[1] = center.d1;
				currGrid = currGrid->m_child[2];
			}
			else
			{
				currScope.m_pLow[1] = center.d1;
				currScope.m_pHigh[0] = center.d0;
				currGrid = currGrid->m_child[0];
			}
		}
		else
		{
			if (locate.d1 < center.d1)
			{
				currScope.m_pLow[0] = center.d0;
				currScope.m_pHigh[1] = center.d1;
				currGrid = currGrid->m_child[3];
			}
			else
			{
				currScope.m_pLow[0] = center.d0;
				currScope.m_pLow[1] = center.d1;
				currGrid = currGrid->m_child[1];
			}
		}
		++ret;

	} while (M_AL::isNodeInRect(except, currScope));

	return ret;
}

bool MultiLayersNW::SpatialIndex::
	isTwoPointInSameLeaf(const MapPos2D& p0, const MapPos2D& p1) const
{
	assert(m_scope.containsPoint(p0) && m_scope.containsPoint(p1));

	Rect currScope(m_scope);
	Grid* currGrid(m_root);
	do 
	{
		if (currGrid->isLeaf())
			return true;

		MapPos2D center(currScope.center());

		if (p0.d0 < center.d0)
		{
			if (p0.d1 < center.d1)
			{
				currScope.m_pHigh[0] = center.d0;
				currScope.m_pHigh[1] = center.d1;
				currGrid = currGrid->m_child[2];
			}
			else
			{
				currScope.m_pLow[1] = center.d1;
				currScope.m_pHigh[0] = center.d0;
				currGrid = currGrid->m_child[0];
			}
		}
		else
		{
			if (p0.d1 < center.d1)
			{
				currScope.m_pLow[0] = center.d0;
				currScope.m_pHigh[1] = center.d1;
				currGrid = currGrid->m_child[3];
			}
			else
			{
				currScope.m_pLow[0] = center.d0;
				currScope.m_pLow[1] = center.d1;
				currGrid = currGrid->m_child[1];
			}
		}

	} while (M_AL::isNodeInRect(p1, currScope));

	return false;
}

void MultiLayersNW::SpatialIndex::
	getAllScopeByPostorderTraversal(std::vector<Rect>& rects) const
{
	m_root->postorderTraversalFetchScope(m_scope, rects);
}

void MultiLayersNW::SpatialIndex::Grid::
	postorderTraversalFetchScope(const Rect& myScope, std::vector<Rect>& rects)
{
	if (!isLeaf())
	{
		for (size_t i = 0; i < 4; ++i)
		{
			Rect childScope;
			childScope.makeInfinite();
			Tools::SpatialMath::getRectChildGrid(myScope, i, childScope);
			m_child[i]->postorderTraversalFetchScope(childScope, rects);
		}
	}
	
	rects.push_back(myScope);
}

//
// class MultiLayersNW::ComeInNodesLookUpTable
//

void MultiLayersNW::ComeInNodesLookUpTable::
	initByParseStorage(IStorageManager* storage, size_t pageSize, size_t tablePageStart)
{
	m_storage = storage;

	m_pageStart = tablePageStart;

	byte* data;
	size_t len;
	storage->loadByteArray(tablePageStart, len, &data);
	byte* ptr = data;

	do 
	{
		size_t maxSingleNodeIDInPage = 0;
		memcpy(&maxSingleNodeIDInPage, ptr, sizeof(LAYER_1_INDEX::ITEM_SIZE));
		ptr += LAYER_1_INDEX::ITEM_SIZE;
		assert(maxSingleNodeIDInPage > (m_index1.empty() ? 0 : m_index1.back()));
		m_index1.push_back(maxSingleNodeIDInPage);

	} while (m_index1.back() != LAYER_1_INDEX::END_FLAG);
	m_index1.pop_back();

	delete[] data;
}

size_t MultiLayersNW::ComeInNodesLookUpTable::
	mapSingleIDAndLayerToMultiID(size_t singleID, size_t layer) const
{
	size_t index2Page = m_pageStart + LAYER_1_INDEX::PAGE_COUNT + queryFromIndex1(singleID);
	size_t addr = queryFromIndex2(index2Page, singleID);
	if (addr == 0)
		return 0;
	else
		return queryFromData(addr, layer);
}

size_t MultiLayersNW::ComeInNodesLookUpTable::
	queryFromIndex1(size_t singleID) const
{
	std::vector<size_t>::const_iterator itr 
		= std::lower_bound(m_index1.begin(), m_index1.end(), singleID);
	assert(itr != m_index1.end());
	return static_cast<size_t>(itr - m_index1.begin());
}

size_t MultiLayersNW::ComeInNodesLookUpTable::
	queryFromIndex2(size_t pageID, size_t singleID) const
{
	byte* data;
	size_t len;
	m_storage->loadByteArray(pageID, len, &data);
	assert(len > 0 && len % (LAYER_2_INDEX::LAYER2_INDEX_ITEM_SIZE) == 0);

	byte* ptr = data;
	size_t id = 0, addr = 0;
	int lower = 0, upper = len / LAYER_2_INDEX::LAYER2_INDEX_ITEM_SIZE - 1;
	while (lower <= upper)
	{
		int mid = ((lower + upper) >> 1);
		ptr = data + LAYER_2_INDEX::LAYER2_INDEX_ITEM_SIZE * mid;
		memcpy(&id, ptr, sizeof(SINGLE_NODE_ID));

		if (id == singleID)
		{
			ptr += sizeof(SINGLE_NODE_ID);
			memcpy(&addr, ptr, sizeof(ADDRESS));
			return addr;
		}
		else if (id < singleID)
			lower = mid + 1;
		else
			upper = mid - 1;
	}

	return 0;
}

size_t MultiLayersNW::ComeInNodesLookUpTable::
	queryFromData(size_t addr, size_t layer) const
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(addr, &page, &offset);

	byte* data;
	size_t len;
	m_storage->loadByteArray(page, len, &data);
	byte* ptr = data + offset;

	size_t startLayer = 0;
	memcpy(&startLayer, ptr, ENTRY::START_LAYER_SIZE);
	ptr += ENTRY::START_LAYER_SIZE;

	if (layer >= startLayer)
		ptr += sizeof(MULTI_NODE_ID) * (layer - startLayer);

	size_t multiNodeID = 0;
	memcpy(&multiNodeID, ptr, sizeof(MULTI_NODE_ID));

	delete[] data;

	return multiNodeID;
}

//
// Internal
//

void MultiLayersNW::parseRouteResult(const std::vector<std::pair<NODE_ID, NEAREST_DIS> >& route, 
									 const BindPoint& s, const BindPoint& e, 
									 SimulateNavigate::SingleRouteSearchingInfo& info) const
{
	if (route.empty())
	{
		if (s.m_roadID == e.m_roadID)
			info.m_roadIDs.push_back(s.m_roadID);
		return;
	}

	assert(route.size() > 1);

	assert(route.front().first == e.m_beginID || route.front().first == e.m_endID
		&& route.back().first == s.m_beginID || route.back().first == s.m_endID);
	if (route.front().first == e.m_beginID)
		info.m_end.m_bDir = true;
	else
		info.m_end.m_bDir = false;
	if (route.back().first == s.m_beginID)
		info.m_start.m_bDir = true;
	else
		info.m_start.m_bDir = false;

	info.m_roadIDs.push_back(s.m_roadID);

	size_t iCurrID = route.size() - 1;
	while (iCurrID != 0)
	{
		size_t sNodeID = route[iCurrID].first,
			eNodeID = route[iCurrID - 1].first;

		if (sNodeID == eNodeID)
		{
			if (route.size() == 2)
				break;
			else
				assert(0);
		}

		size_t nearestDis = route[iCurrID - 1].second - route[iCurrID].second;

		std::vector<S_CONN> conns;
		m_singleNW->getTopoNodeConnection(sNodeID, &conns);

		size_t nearbyDis = Tools::UINT32_MAX;
		uint64_t roadID;
		for (size_t i = 0; i < conns.size(); ++i)
		{
			const S_CONN& ct = conns[i];
			if (ct.nodeID != eNodeID)
				continue;
			else if (ct.length < nearbyDis)
			{
				nearbyDis = ct.length;
				roadID = ct.roadID;
			}
		}

		if (nearbyDis > nearestDis)
		{
			BetweenTwoNodeSearchingInfo part(sNodeID, eNodeID, nearestDis);
			m_singleNW->computeShortestPath(part);
			std::copy(part.m_roadIDs.begin(), part.m_roadIDs.end(), back_inserter(info.m_roadIDs));
		}
		else
		{
			assert(nearbyDis == nearestDis);
			info.m_roadIDs.push_back(roadID);
		}

		--iCurrID;
	}

	info.m_roadIDs.push_back(e.m_roadID);
}

void MultiLayersNW::parseHeader(size_t& pageSize, size_t& indexPageCount, size_t& tablePageStart)
{
	byte* data;
	size_t len;
	m_buffer->loadByteArray(NVDataPublish::MLNW::Publish::HEADER_PAGE, len, &data);
	byte* ptr = data;

	memcpy(&pageSize, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&indexPageCount, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&tablePageStart, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	delete[] data;
}
