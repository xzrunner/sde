#include "SingleLayerNW.h"
#include "BindPoint.h"
#include "SLSingleSrcSearching.h"
#include "SLMultiSrcSearching.h"
#include "../../NVDataPublish/Network/Node.h"
#include "../../NVDataPublish/Network/Leaf.h"
#include "../../NVDataPublish/Network/Publish.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

typedef NVDataPublish::Network::AdjacencyList	AL;
typedef NVDataPublish::Network::Node			PM_NODE;

SingleLayerNW::SingleLayerNW(IStorageManager* sm, const SimulateNavigate::IPointBindLine* bind,
							 const NVDataPublish::ILookUp* table, bool bPreLoadTable)
	: m_buffer(sm), m_bind(bind), m_table(table), m_bPreLoadTable(bPreLoadTable)
{
	m_scope.makeInfinite();
	parseHeader();
}

SingleLayerNW::SingleLayerNW(IStorageManager* sm)
	: m_buffer(sm), m_bind(NULL), m_table(NULL), m_bPreLoadTable(false)
{
	m_scope.makeInfinite();
	parseHeader();
}

double SingleLayerNW::getShortestDistance(size_t src, size_t dest) const
{
	SLSingleSrcSearching search(this, m_table, m_bPreLoadTable);
	return search.queryShortestDistance(src, dest);
}

double SingleLayerNW::computeShortestPath(SimulateNavigate::BetweenTwoNodeSearchingInfo& info) const
{
	SLSingleSrcSearching search(this, info.m_disPro, m_table, m_bPreLoadTable);

	std::vector<size_t> routeNodeIDs;
	double dis = search.queryShortestRoute(info.m_start, info.m_end, &routeNodeIDs, info.m_knownDis);

	parseRouteResult(routeNodeIDs, info);

	return dis;
}

double SingleLayerNW::computeShortestPath(SimulateNavigate::SingleRouteSearchingInfo& info) const
{
	BindPoint s(info.m_start.m_realPos, m_bind, this), 
		e(info.m_end.m_realPos, m_bind, this);
	info.m_start.m_bindPos = s.m_pos;
	info.m_end.m_bindPos = e.m_pos;
	std::vector<size_t> routeNodeIDs;

	double dis;
	SLSingleSrcSearching search(this, info.m_disPro, m_table, m_bPreLoadTable, info.m_bAStar);
	dis = search.queryShortestRoute(s, e, &routeNodeIDs, &info.m_expandNum);

	parseRouteResult(routeNodeIDs, s, e, info);

	return dis;
}

void SingleLayerNW::computeShortestPath(SimulateNavigate::MultiRoutesSearchingInfo& info) const
{
	std::vector<std::vector<size_t> > multiRouteNodeIDs;

	SLMultiSrcSearching search(this);
	search.queryShortestRoute(info.m_multiSrc, info.m_scope, &multiRouteNodeIDs);

	parseRouteResult(multiRouteNodeIDs, info);
}

size_t SingleLayerNW::queryTopoNodeID(const MapPos2D& p) const
{
	std::vector<size_t> IDs;
	retrieveIndex(p, &IDs);

	for (size_t i = 0; i < IDs.size(); ++i)
	{
		byte* data;
		size_t len;
		m_buffer->loadByteArray(IDs.at(i), len, &data);

		size_t offset = AL::computeDataOffset(p, data);
		delete[] data;
		if (offset != 0)
			return AL::encodeTopoNodeID(IDs.at(i), offset);
	}

	assert(0);
	throw Tools::IllegalStateException("SingleLayerNW::queryTopoNodeID: Can't find pos.");
}

void SingleLayerNW::getTopoNodePos(size_t nodeID, MapPos2D* pos) const
{
	size_t page, offset;
	AL::decodeTopoNodeID(nodeID, &page, &offset);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	AL::fetchCoordinate(ptr, pos);

	delete[] data;
}

size_t SingleLayerNW::getConnectionSetID(size_t nodeID) const
{
	size_t page, offset;
	AL::decodeTopoNodeID(nodeID, &page, &offset);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	size_t id = AL::fetchConnectionSetID(ptr);

	delete[] data;
	return id;
}

void SingleLayerNW::getTopoNodeConnection(size_t nodeID, std::vector<S_CONN>* conns,
										  bool bUsedInMLSearch /*= false*/) const
{
	size_t page, offset;
	AL::decodeTopoNodeID(nodeID, &page, &offset);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	AL::fetchConnectionInfo(ptr, conns);

	delete[] data;

	if (!bUsedInMLSearch)
	{
		for (size_t i = 0; i < conns->size(); ++i)
			(*conns)[i].nodeID &= AL::TOPO_NODE_GAP;
	}
}

size_t SingleLayerNW::getTopoNodeSize() const
{
	if (m_rootBeLeaf)
	{		
		byte* data;
		size_t len;
		m_buffer->loadByteArray(m_firstLeafPage, len, &data);

		size_t startALPageID;
		size_t ALCount;
		NVDataPublish::Network::Leaf::getAllAdjacencyListIDs(
			data + NVDataPublish::Network::Publish::LEAF_COUNT_SIZE, startALPageID, ALCount
			);

		delete[] data;

		size_t count = 0;
		for (size_t i = 0; i < ALCount; ++i)
		{
			m_buffer->loadByteArray(startALPageID, len, &data);
			count += AL::fetchNodeCount(data);
			delete[] data;
			++startALPageID;
		}

		return count;
	}
	else
	{
		size_t count = 0;

		byte* data;
		size_t len;
		m_buffer->loadByteArray(m_firstIndexPage, len, &data);

		byte* ptr = data;
		for (size_t i = 0; i < 4; ++i)
		{
			ptr += PM_NODE::PM_NODE_ID_SIZE;
			size_t c = 0;
			memcpy(&c, ptr, PM_NODE::PM_NODE_COUNT_SIZE);
			ptr += PM_NODE::PM_NODE_COUNT_SIZE;

			count += c;
		}

		delete[] data;

		return count;
	}
}

void SingleLayerNW::test() const
{
	std::vector<uint64_t> routeID;
	MapPos2D sBind, eBind;

	clock_t clock0, clock1;

	std::ifstream fin("testData.txt");
	std::ofstream ftime("result.txt"), fnodeID("nodeID.txt");

	MapPos2D start, end;
	size_t num = 0;
	while (fin)
	{
		fin >> start.d0 >> start.d1 >> end.d0 >> end.d1;

		clock0 = clock(); 
		BindPoint s(start, m_bind, this), e(end, m_bind, this);
		clock1 = clock();
		ftime << "[" << num++ << "]: " << "Prepare: " << clock1 - clock0;

		std::vector<size_t> routeNodeID;

		SLSingleSrcSearching search(this, m_table, m_bPreLoadTable);
		search.queryShortestRoute(s, e, &routeNodeID);

		clock0 = clock();
		ftime << "   Search: " << clock0 - clock1 << std::endl;

		s.output(fnodeID);
		e.output(fnodeID);
	}
}

void SingleLayerNW::parseHeader()
{
	byte* data;
	size_t len;
	m_buffer->loadByteArray(NVDataPublish::Network::Publish::HEADER_PAGE, len, &data);
	byte* ptr = data;

	memcpy(m_scope.m_pLow, ptr, sizeof(double) * 2);
	ptr += sizeof(double) * 2;
	memcpy(m_scope.m_pHigh, ptr, sizeof(double) * 2);
	ptr += sizeof(double) * 2;

	size_t bLeaf = 0;
	memcpy(&bLeaf, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	m_rootBeLeaf = bLeaf == 0 ? false : true;

	memcpy(&m_firstLeafPage, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_firstIndexPage, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	delete[] data;
}

void SingleLayerNW::parseRouteResult(const std::vector<size_t>& nodeIDs, 
									 SimulateNavigate::BetweenTwoNodeSearchingInfo& info) const
{
	if (nodeIDs.size() < 2)
		return;

	size_t iCurrID = nodeIDs.size() - 1;
	do
	{
		size_t sNodeID = nodeIDs[iCurrID],
			eNodeID = nodeIDs[iCurrID - 1];

		if (sNodeID == eNodeID)
		{
			if (nodeIDs.size() == 2)
				break;
			else
				assert(0);
		}

		std::vector<S_CONN> conns;
		getTopoNodeConnection(sNodeID, &conns);
		double dis = Tools::DOUBLE_MAX;
		uint64_t roadID;
		for (size_t i = 0; i < conns.size(); ++i)
		{
			const S_CONN& ct = conns[i];
			if (ct.nodeID != eNodeID)
				continue;
			else if (ct.length < dis)
			{
				dis = ct.length;
				roadID = ct.roadID;
			}
		}

		assert(dis != Tools::DOUBLE_MAX);
		info.m_roadIDs.push_back(roadID);

		--iCurrID;

	} while (iCurrID != 0);
}

void SingleLayerNW::parseRouteResult(const std::vector<size_t>& nodeIDs, const BindPoint& s, const BindPoint& e,
									 SimulateNavigate::SingleRouteSearchingInfo& info) const
{
	if (nodeIDs.empty())
	{
		if (s.m_roadID == e.m_roadID)
			info.m_roadIDs.push_back(s.m_roadID);
		return;
	}

	assert(nodeIDs.size() > 1);

	assert(nodeIDs.front() == e.m_beginID || nodeIDs.front() == e.m_endID
		&& nodeIDs.back() == s.m_beginID || nodeIDs.back() == s.m_endID);
	if (nodeIDs.front() == e.m_beginID)
		info.m_end.m_bDir = true;
	else
		info.m_end.m_bDir = false;
	if (nodeIDs.back() == s.m_beginID)
		info.m_start.m_bDir = true;
	else
		info.m_start.m_bDir = false;

	info.m_roadIDs.push_back(s.m_roadID);

	size_t iCurrID = nodeIDs.size() - 1;
	while (iCurrID != 0)
	{
		size_t sNodeID = nodeIDs[iCurrID],
			eNodeID = nodeIDs[iCurrID - 1];

		if (sNodeID == eNodeID)
		{
			if (nodeIDs.size() == 2)
				break;
			else
				assert(0);
		}

		std::vector<S_CONN> conns;
		getTopoNodeConnection(sNodeID, &conns);
		double dis = Tools::DOUBLE_MAX;
		uint64_t roadID;
		for (size_t i = 0; i < conns.size(); ++i)
		{
			const S_CONN& ct = conns[i];
			if (ct.nodeID != eNodeID)
				continue;
			else if (ct.length < dis)
			{
				dis = ct.length;
				roadID = ct.roadID;
			}
		}

		assert(dis != Tools::DOUBLE_MAX);
		info.m_roadIDs.push_back(roadID);

		--iCurrID;
	}

	info.m_roadIDs.push_back(e.m_roadID);
}

void SingleLayerNW::parseRouteResult(std::vector<std::vector<size_t> > multiRouteNodeIDs, 
									 SimulateNavigate::MultiRoutesSearchingInfo& info) const
{
	BetweenTwoNodeSearchingInfo tmpInfo;
	for (size_t i = 0; i < multiRouteNodeIDs.size(); ++i)
	{
		tmpInfo.m_roadIDs.clear();
		parseRouteResult(multiRouteNodeIDs[i], tmpInfo);
		for (size_t j = 0; j < tmpInfo.m_roadIDs.size(); ++j)
			info.m_roadIDs.insert(tmpInfo.m_roadIDs[j]);
	}
}

void SingleLayerNW::retrieveIndex(const MapPos2D& p, std::vector<size_t>* IDs) const
{
	if (m_rootBeLeaf)
		return retrieveLeaf(p, m_firstLeafPage, NVDataPublish::Network::Publish::LEAF_COUNT_SIZE, m_scope, IDs);

	byte* data;
	size_t len;
	m_buffer->loadByteArray(m_firstIndexPage, len, &data);
	byte* ptr = data;

	Rect scope = m_scope;
	while (true)
	{
		MapPos2D center(scope.center());
		size_t position;
		if (p.d0 < center.d0)
		{
			if (p.d1 < center.d1)
			{
				position = 2;
				scope.m_pHigh[0] = center.d0;
				scope.m_pHigh[1] = center.d1;
			}
			else
			{
				position = 0;
				scope.m_pLow[1] = center.d1;
				scope.m_pHigh[0] = center.d0;
			}
		}
		else
		{
			if (p.d1 < center.d1)
			{
				position = 3;
				scope.m_pLow[0] = center.d0;
				scope.m_pHigh[1] = center.d1;
			}
			else
			{
				position = 1;
				scope.m_pLow[0] = center.d0;
				scope.m_pLow[1] = center.d1;
			}
		}

		ptr += (PM_NODE::PM_NODE_ID_SIZE + PM_NODE::PM_NODE_COUNT_SIZE) * position;
		size_t indexID;
		memcpy(&indexID, ptr, PM_NODE::PM_NODE_ID_SIZE);
		//ptr += sizeof(PM_NODE::PM_NODE_ID_SIZE);

		delete[] data;

		size_t page, offset;
		AL::decodeTopoNodeID(indexID, &page, &offset);

		if (page < m_firstIndexPage)
			return retrieveLeaf(p, page, offset, scope, IDs);
		else
		{
			m_buffer->loadByteArray(page, len, &data);
			ptr = data + offset;
		}
	}

	delete[] data;
	assert(0);
	throw Tools::IllegalStateException("SingleLayerNW::retrieveIndex: Can't find leaf.");
}

void SingleLayerNW::retrieveLeaf(const MapPos2D& p, size_t page, size_t offset, const Rect& s, std::vector<size_t>* IDs) const
{
	assert(s.containsPoint(p) || s.touchesPoint(p));

	byte* data;
	size_t len;
	m_buffer->loadByteArray(page, len, &data);
	byte* ptr = data + offset;

	NVDataPublish::Network::Leaf::queryAdjacencyListID(p, ptr, IDs);

	delete[] data;
}
