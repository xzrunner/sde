#include "TableBuilding.h"
#include "Publish.h"
#include "../Network/AdjacencyList.h"
#include "../MultiLayersNW/AdjacencyList.h"
#include "../../SimulateNavigate/Network/SingleLayerNW.h"
#include "../../SimulateNavigate/Network/MultiLayersNW.h"
#include "../../SimulateNavigate/Network/SearchingUtility.h"
#include "../../NVDataPublish/Network/Publish.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::LookUpTable;

typedef NVDataPublish::Network::AdjacencyList	S_AL;
typedef NVDataPublish::MLNW::AdjacencyList		M_AL;
typedef NVDataPublish::Network::Connection		CONN;
typedef SimulateNavigate::Network::PTR_VISITED	PTR_VISITED;

TableBuilding::~TableBuilding()
{
	for_each(m_grids.begin(), m_grids.end(), Tools::DeletePointerFunctor<Grid>());

	delete m_network;
}

void TableBuilding::build(HWND hwnd, IStorageManager* nwStorage, IStorageManager* tableStorage, 
						  size_t startPage)
{
	m_network = new SimulateNavigate::Network::SingleLayerNW(nwStorage);

	initAllGrids(nwStorage);

	//// method 0: 
	//computeTableDataDirectlySearch(hwnd);
	// method 1:
	computeTableDataFastExpend(hwnd);

	outputToStorage(tableStorage, startPage);
}

double TableBuilding::queryMinDistance(IStorageManager* sm, size_t startPage, 
									   size_t gridNum, size_t sGridID, size_t dGridID)
{
	size_t pos = (dGridID * gridNum + sGridID) * DIS_SIZE;
	size_t page = pos / Publish::PAGE_SIZE,
		offset = pos - page * Publish::PAGE_SIZE;
	page += startPage;

	byte* data;
	size_t len;
	sm->loadByteArray(static_cast<id_type>(page), len, &data);

	uint16_t dis = 0;
	memcpy(&dis, data + offset, DIS_SIZE);
	delete[] data;

	return decodeDis(dis);
}

void TableBuilding::loadTable(IStorageManager* sm, size_t startPage, size_t gridNum,
							  size_t destGridID, byte** data)
{
	size_t pos = destGridID * gridNum * DIS_SIZE;
	size_t page = pos / Publish::PAGE_SIZE,
		offset = pos - page * Publish::PAGE_SIZE;
	page += startPage;

	size_t len = gridNum * DIS_SIZE;
	*data = new byte[len];
	
	byte* ptrDest = *data;
	size_t cLen;
	size_t cRem = len, pageRem;

	do 
	{
		byte* dataSrc;
		size_t lenSrc;
		sm->loadByteArray(static_cast<id_type>(page), lenSrc, &dataSrc);

		byte* ptrSrc = dataSrc + offset;

		pageRem = Publish::PAGE_SIZE - offset;
		cLen = (cRem > pageRem) ? pageRem : cRem;
		memcpy(ptrDest, ptrSrc, cLen);

		ptrDest += cLen;
		cRem -= cLen;
		++page;
		offset = 0;

		delete[] dataSrc;
	} while (cRem != 0);
}

double TableBuilding::queryMinDistance(size_t srcGridID, const byte* table)
{
	uint16_t dis = 0;
	memcpy(&dis, table + srcGridID * DIS_SIZE, DIS_SIZE);
	return decodeDis(dis);
}

void TableBuilding::initAllGrids(IStorageManager* sm)
{
	size_t cGrid = m_lookUp->getGridsCount();
	m_grids.reserve(cGrid);
	for (size_t i = 0; i < cGrid; ++i)
		m_grids.push_back(new Grid(cGrid));
	
	size_t lPage, iPage;
	m_network->getStorageInfo(&lPage, &iPage);
	size_t page = NVDataPublish::Network::Publish::BEGIN_AL_PAGE;
	for ( ; page < lPage; ++page)
	{
		byte* data;
		size_t len;
		sm->loadByteArray(page, len, &data);

		byte* ptr = data;

		size_t cData = 0;
		memcpy(&cData, ptr, S_AL::DATA_COUNT_SIZE);
		ptr += S_AL::DATA_COUNT_SIZE;
		for (size_t iData = 0; iData < cData; ++iData)
		{
			size_t fNodeID = S_AL::encodeTopoNodeID(page, ptr - data);

			MapPos2D fPos;
			S_AL::fetchCoordinate(ptr, &fPos);
			ptr += S_AL::COORD_SIZE * 2 + S_AL::CONNECTION_ID_SIZE;

			size_t fGridID = m_lookUp->getGridID(fPos);
			assert(fGridID < cGrid);

			size_t cEdge = 0;
			memcpy(&cEdge, ptr, S_AL::DATA_COUNT_SIZE);
			ptr += S_AL::DATA_COUNT_SIZE;
			for (size_t iEdge = 0; iEdge < cEdge; ++iEdge)
			{
				size_t tNodeID = 0;
				memcpy(&tNodeID, ptr, S_AL::NODE_ID_SIZE);
				size_t tPage, tOffset;
				S_AL::decodeTopoNodeID(tNodeID, &tPage, &tOffset);

				byte* tData;
				size_t tLen;
				sm->loadByteArray(tPage, tLen, &tData);
				byte* tPtr = tData + tOffset;
				MapPos2D tPos;
				S_AL::fetchCoordinate(tPtr, &tPos);
				delete[] tData;

				size_t tGridID = m_lookUp->getGridID(tPos);
				assert(tGridID < cGrid);

				// todo: the pressed coordinate may incur some error
				// ex. located in the wrong Grid
				if (tGridID != fGridID)
				{
					m_grids[fGridID]->m_outNode.push_back(fNodeID);
					m_grids[tGridID]->m_inNode.push_back(tNodeID & M_AL::TOPO_NODE_GAP);
				}

				ptr += S_AL::CONNECTION_ITEM_SIZE;
			}
		}

		delete[] data;
	}

	uniqueGridsData();
	initPairNodeToGrid();
}

void TableBuilding::uniqueGridsData()
{
	std::vector<size_t>::iterator itr;
	for (size_t i = 0; i < m_grids.size(); ++i)
	{
		if (!m_grids[i]->m_outNode.empty())
		{
			itr = std::unique(m_grids[i]->m_outNode.begin(), m_grids[i]->m_outNode.end());
			m_grids[i]->m_outNode.resize(itr - m_grids[i]->m_outNode.begin());
		}

		if (!m_grids[i]->m_inNode.empty())
		{
			itr = std::unique(m_grids[i]->m_inNode.begin(), m_grids[i]->m_inNode.end());
			m_grids[i]->m_inNode.resize(itr - m_grids[i]->m_inNode.begin());
		}
	}
}

void TableBuilding::initPairNodeToGrid()
{
	size_t cInNode = 0;
	for (size_t i = 0; i < m_grids.size(); ++i)
		cInNode += m_grids[i]->m_inNode.size();
	m_inNodeToGrid.reserve(cInNode);

	for (size_t i = 0; i < m_grids.size(); ++i)
		for (size_t j = 0; j < m_grids[i]->m_inNode.size(); ++j)
			m_inNodeToGrid.push_back(std::make_pair(m_grids[i]->m_inNode[j], i));

	std::sort(m_inNodeToGrid.begin(), m_inNodeToGrid.end(), pairNodeGridCmp);

	//for (size_t i = 0; i < m_grids.size(); ++i)
	//	for (size_t j = 0; j < m_grids[i]->m_inNode.size(); ++j)
	//		m_inNodeToGrid.insert(std::make_pair(m_grids[i]->m_inNode[j], i));
}

void TableBuilding::computeTableDataDirectlySearch(HWND hwnd)
{
	for (size_t iFromGrid = 0; iFromGrid < m_grids.size(); ++iFromGrid)
	{
		// display progress
		char positionBuffer[100];
		sprintf(positionBuffer, "%d / %d", iFromGrid, m_grids.size());
		HDC hdc = GetDC(hwnd);
		TextOutA(hdc, 10, 0, positionBuffer, strlen(positionBuffer));
		ReleaseDC(hwnd, hdc);

		for (size_t iFromNode = 0; iFromNode < m_grids[iFromGrid]->m_outNode.size(); ++iFromNode)
		{
			size_t fNode = m_grids[iFromGrid]->m_outNode[iFromNode];
			
			for (size_t iToGrid = 0; iToGrid < m_grids.size(); ++iToGrid)
			{
				if (iToGrid == iFromGrid)
					continue;
				for (size_t iToNode = 0; iToNode < m_grids[iToGrid]->m_inNode.size(); ++iToNode)
				{
					size_t tNode = m_grids[iToGrid]->m_inNode[iToNode];
					double dis = m_network->getShortestDistance(fNode, tNode);
					if (dis < m_grids[iFromGrid]->m_minDisToOthers[iToGrid])
						m_grids[iFromGrid]->m_minDisToOthers[iToGrid] = dis;
				}
			}
		}
	}
}

void TableBuilding::computeTableDataFastExpend(HWND hwnd)
{
	// debug
	std::ofstream fout("c:/3_24_table.txt");
	//
	size_t cGrid = m_grids.size();
	for (size_t i = 0; i < cGrid; ++i)
	{
		// debug
		fout << i << " / " << cGrid << std::endl;
		//

		// display progress
		char positionBuffer[100];
		sprintf(positionBuffer, "%d / %d", i, cGrid);
		HDC hdc = GetDC(hwnd);
		TextOutA(hdc, 10, 0, positionBuffer, strlen(positionBuffer));
		ReleaseDC(hwnd, hdc);

		Grid* grid = m_grids[i];
		for (size_t j = 0; j < grid->m_outNode.size(); ++j)
		{
			size_t fNode = grid->m_outNode[j];

			VisitedList visited;
			CandidateList candidate;

			VisitedNode* b = new VisitedNode(fNode, 0.0, NULL);
			visited.push(b);
			candidate.push(b);

			while (!candidate.empty())
			{
				VisitedNode* opt = candidate.top(); candidate.pop();

				//std::map<TOPO_NODE_ID, TABLE_GRID_ID>::iterator itr 
				//	= m_inNodeToGrid.find(opt->m_nodeID);
				//if (itr != m_inNodeToGrid.end())
				//	updateMinDis(fNode, opt->m_from, itr->second);

				std::vector<INNODE_TO_GRID>::iterator itr
					= std::lower_bound(m_inNodeToGrid.begin(), m_inNodeToGrid.end(), 
					INNODE_TO_GRID(opt->m_nodeID, Tools::UINT32_MAX), pairNodeGridCmp);
				if (itr != m_inNodeToGrid.end() && itr->first == opt->m_nodeID)
					updateMinDis(fNode, opt->m_from, itr->second);

				circleExpand(opt, visited, candidate);
			}
		}
	}
}

void TableBuilding::updateMinDis(size_t fNode, double dis, size_t tGridID)
{
	MapPos2D fPos;
	m_network->getTopoNodePos(fNode, &fPos);
	size_t fGridID = m_lookUp->getGridID(fPos);
	if (fGridID != tGridID && dis < m_grids.at(fGridID)->m_minDisToOthers.at(tGridID))
		m_grids.at(fGridID)->m_minDisToOthers.at(tGridID) = dis;
}

void TableBuilding::circleExpand(VisitedNode* n, VisitedList& visited, CandidateList& candidate) const
{
	std::vector<CONN> conns;
	m_network->getTopoNodeConnection(n->m_nodeID, &conns);
	for (size_t i = 0; i < conns.size(); ++i)
	{
		const CONN& ct = conns.at(i);

		PTR_VISITED pVisited = visited.find(ct.nodeID);
		if (visited.exist(pVisited))
		{
			double newFrom = n->m_from + ct.length;
			if (newFrom < (*pVisited)->m_from)
				candidate.update(*pVisited, newFrom, n);
		}
		else
		{
			VisitedNode* newNode = new VisitedNode(ct.nodeID, n->m_from + ct.length, n);
			visited.push(newNode);
			candidate.push(newNode);
		}
	}
}

void TableBuilding::outputToStorage(IStorageManager* sm, size_t startPage) const
{
	id_type currPage = startPage;
	byte* data = new byte[Publish::PAGE_SIZE];
	byte* ptr = data;

	for (size_t iDest = 0; iDest < m_grids.size(); ++iDest)
	{
		for (size_t iSrc = 0; iSrc < m_grids.size(); ++iSrc)
		{
			uint16_t dis;
			if (iDest == iSrc)
				dis = 0;
			else
				dis = encodeDis(m_grids[iSrc]->m_minDisToOthers[iDest]);

			memcpy(ptr, &dis, DIS_SIZE);
			ptr += DIS_SIZE;

			if (ptr - data == Publish::PAGE_SIZE)
			{
				sm->storeByteArray(currPage, Publish::PAGE_SIZE, data);
				++currPage;
				ptr = data;
			}
		}
	}

	if (ptr - data != 0)
		sm->storeByteArray(currPage, Publish::PAGE_SIZE, data);

	delete[] data;
}

uint16_t TableBuilding::encodeDis(double len)
{
	if (len == Tools::DOUBLE_MAX)
		return MAX_DIS;

	size_t intLen = static_cast<size_t>(len);
	if (intLen <= DIS_16KM)
		return static_cast<uint16_t>(intLen);
	else if (intLen <= DIS_128KM)
		return (static_cast<uint16_t>(intLen >> 3)) | DIS_128CARRY;
	else if (intLen <= DIS_1024KM)
		return (static_cast<uint16_t>(intLen >> 6)) | DIS_1024CARRY;
	else
	{
		if (intLen >= DIS_8192KM)
		{
			assert(0);
			throw Tools::IllegalArgumentException(
				"TableBuilding::encodeLength: over the bound!"
				);
		}
		return (static_cast<uint16_t>(intLen >> 9)) | DIS_8192CARRY;
	}
}

size_t TableBuilding::decodeDis(uint16_t len)
{
	if (len <= DIS_16GAP)
		return len;
	else if (len <= DIS_128GAP)
		return static_cast<size_t>(len ^ DIS_128CARRY) << 3;
	else if (len <= DIS_1024GAP)
		return static_cast<size_t>(len ^ DIS_1024CARRY) << 6;
	else if (len < DIS_8192GAP)
		return static_cast<size_t>(len ^ DIS_8192CARRY) << 9;
	else
		return MAX_DIS;
}