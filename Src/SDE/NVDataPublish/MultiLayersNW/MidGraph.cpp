#include "MidGraph.h"
#include "Node.h"
#include "../Network/AdjacencyList.h"
#include "../../Tools/SpatialMath.h"
#include "../../SimulateNavigate/Network/SearchingUtility.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW;

typedef NVDataPublish::Network::AdjacencyList		S_AL;
typedef AdjacencyList								M_AL;

typedef SimulateNavigate::Network::VisitedList		VisitedList;
typedef SimulateNavigate::Network::CandidateList	CandidateList;
typedef SimulateNavigate::Network::VisitedNode		VisitedNode;
typedef SimulateNavigate::Network::PTR_VISITED		PTR_VISITED;

MidGraph::~MidGraph()
{
	SRC_MAP::iterator itr = m_srcNodes.begin();
	for ( ; itr != m_srcNodes.end(); ++itr)
		delete itr->second;
	delete m_destGraph;
}

void MidGraph::initFromSingleNWStorage(IStorageManager& sm, const std::vector<size_t>& gridIDs)
{
	for (size_t i = 0; i < gridIDs.size(); ++i)
	{
		size_t page = gridIDs[i];
		byte* data;
		size_t len;
		sm.loadByteArray(page, len, &data);

		byte* ptr = data;
		bool bDirty = false;

		size_t cData = 0;
		memcpy(&cData, ptr, S_AL::DATA_COUNT_SIZE);
		ptr += S_AL::DATA_COUNT_SIZE;
		for (size_t iData = 0; iData < cData; ++iData)
		{
			LOW_ID fNodeID = S_AL::encodeTopoNodeID(page, ptr - data);

			MapPos2D fPos;
			S_AL::fetchCoordinate(ptr, &fPos);
			ptr += S_AL::COORD_SIZE * 2 + S_AL::CONNECTION_ID_SIZE;

			size_t cEdge = 0;
			memcpy(&cEdge, ptr, S_AL::DATA_COUNT_SIZE);
			ptr += S_AL::DATA_COUNT_SIZE;
			if (M_AL::isNodeInRect(fPos, m_grid->m_scope))
			{
				TopoNode* topoNode = new TopoNode(fNodeID);

				for (size_t iEdge = 0; iEdge < cEdge; ++iEdge)
				{
					LOW_ID tNodeID;
					memcpy(&tNodeID, ptr, S_AL::NODE_ID_SIZE);

					Connection conn;

					MapPos2D tPos;
					M_AL::getTopoNodePos(sm, tNodeID, &tPos);
					assert((tNodeID & M_AL::TOPO_NODE_CARRY) == 0);
					if (!M_AL::isNodeInRect(tPos, m_grid->m_scope))
					{
						m_leaveGridNodeIDs.insert(fNodeID);
						conn.nodeID = (M_AL::TOPO_NODE_CARRY | tNodeID);
						memcpy(ptr, &conn.nodeID, S_AL::NODE_ID_SIZE);
						bDirty = true;
					}
					else
						conn.nodeID = tNodeID;

					ptr += S_AL::NODE_ID_SIZE + S_AL::EDGE_ID_SIZE;

					size_t len = 0;
					memcpy(&len, ptr, S_AL::EDGE_LENGTH_SIZE);
					ptr += S_AL::EDGE_LENGTH_SIZE;
					conn.length = S_AL::decodeTopoEdgeLength(len);

					topoNode->m_conns.push_back(conn);
				}
				m_srcNodes.insert(std::make_pair(fNodeID, topoNode));
			}
			else
				ptr += S_AL::CONNECTION_ITEM_SIZE * cEdge;
		}

		if (bDirty)
		{
			id_type pageID = page;
			sm.storeByteArray(pageID, len, data);
		}

		delete[] data;
	}
}

void MidGraph::initFromMultiNWStorage(IStorageManager& singleSM, IStorageManager& multiSM, 
									  const std::vector<size_t>& gridIDs)
{
	for (size_t i = 0; i < gridIDs.size(); ++i)
	{
		size_t page = gridIDs[i];
		byte* data;
		size_t len;
		multiSM.loadByteArray(page, len, &data);

		byte* ptr = data;

		size_t cData = 0;
		memcpy(&cData, ptr, M_AL::DATA_COUNT_SIZE);
		ptr += M_AL::DATA_COUNT_SIZE;
		for (size_t iData = 0; iData < cData; ++iData)
		{
			size_t fNodeID = S_AL::encodeTopoNodeID(page, ptr - data);

			MapPos2D fPos;
			LOW_ID fLowID;
			memcpy(&fLowID, ptr, M_AL::SINGLE_TOPO_NODE_ID_SIZE);
			ptr += M_AL::SINGLE_TOPO_NODE_ID_SIZE;
			M_AL::getTopoNodePos(singleSM, fLowID, &fPos);

			size_t cEdge = 0;
			memcpy(&cEdge, ptr, M_AL::EDGE_COUNT_SIZE);
			ptr += M_AL::EDGE_COUNT_SIZE;
			if (M_AL::isNodeInRect(fPos, m_grid->m_scope))
			{
				TopoNode* topoNode = new TopoNode(fLowID);

				for (size_t iEdge = 0; iEdge < cEdge; ++iEdge)
				{
					size_t tNodeID;
					memcpy(&tNodeID, ptr, M_AL::MULTI_TOPO_NODE_ID_SIZE);
					ptr += M_AL::MULTI_TOPO_NODE_ID_SIZE;

					Connection conn;

					if (tNodeID & M_AL::TOPO_NODE_CARRY)
					{
#ifndef NDEBUG
						MapPos2D tPos;
						M_AL::getTopoNodePos(singleSM, multiSM, tNodeID, &conn.nodeID, &tPos);
						if (!M_AL::isNodeInRect(tPos, m_grid->m_scope))
						{
							assert(0);
							throw Tools::IllegalStateException("Should in the grid.");
						}
#else
						M_AL::getSingleTopoNodeID(multiSM, tNodeID, &conn.nodeID);
#endif
					}
					else
					{
						MapPos2D tPos;
						M_AL::getTopoNodePos(singleSM, tNodeID, &tPos);
						if (!M_AL::isNodeInRect(tPos, m_grid->m_scope))
						{
							m_leaveGridNodeIDs.insert(fLowID);
							conn.nodeID = (M_AL::TOPO_NODE_CARRY | tNodeID);
						}
						else
							conn.nodeID = tNodeID;
					}

					memcpy(&conn.length, ptr, M_AL::EDGE_LENGTH_SIZE);
					ptr += M_AL::EDGE_LENGTH_SIZE;

					topoNode->m_conns.push_back(conn);
				}
				m_srcNodes.insert(std::make_pair(fLowID, topoNode));
			}
			else
				ptr += M_AL::CONNECTION_ITEM_SIZE * cEdge;
		}

		delete[] data;
	}
}

void MidGraph::condenseBySearchingShortestPath() const
{
	std::map<SINGLE_NODE_ID, MULTI_NODE_ID>::const_iterator itr 
		= m_grid->m_transEnterGridNodeIDs.begin();
	for ( ; itr != m_grid->m_transEnterGridNodeIDs.end(); ++itr)
	{
		SRC_MAP::const_iterator itrExist = m_srcNodes.find(itr->first);
		if (itrExist != m_srcNodes.end())
			expandInGrid(itr->first);
	}
	m_destGraph->condense();
}

void MidGraph::outputCondensedGraph(IStorageManager& sm, size_t pageSize, id_type& nextPage)
{
	std::vector<std::vector<CondensedTopoNode*> > dataPages;
	m_destGraph->setAllNodesID(pageSize, nextPage, dataPages);
	m_destGraph->setEnterGridNodeIDTrans(m_grid);
	m_destGraph->outputToStorage(sm, pageSize, nextPage, dataPages);
}

//
// class MidGraph::CondensedGraph
//

void MidGraph::CondensedGraph::insertRouteReversed(const std::vector<LOW_ID>& route, const SRC_MAP& allNodes)
{
	for (int i = route.size() - 1; i >= 0; --i)
	{
		LOW_ID currID = route[i];

		CondensedTopoNode* destNode;
		DEST_MAP::iterator itr = m_destNodes.find(currID);
		if (itr == m_destNodes.end())
		{
			assert(allNodes.find(currID) != allNodes.end());
			destNode = new CondensedTopoNode(currID);
			m_destNodes.insert(std::make_pair(currID, destNode));
		}
		else
			destNode = itr->second;

		if (i == 0)
		{
			SRC_MAP::const_iterator itrCurr = allNodes.find(currID);
			assert(itrCurr != allNodes.end());
			const TopoNode* leaveNode = itrCurr->second;

			bool bFind = false;
			for (size_t j = 0; j < leaveNode->m_conns.size(); ++j)
			{
				const Connection& conn(leaveNode->m_conns[j]);
				if (conn.nodeID & M_AL::TOPO_NODE_CARRY)
				{
					bFind = true;
					destNode->m_out.insert(std::make_pair(conn.nodeID, conn.length));
				}
			}
			assert(bFind);
		}
		if (i != route.size() - 1)
		{
			LOW_ID fID = route[i + 1];
			SRC_MAP::const_iterator itr = allNodes.find(fID);
			assert(itr != allNodes.end());
			TopoNode* fNode = itr->second;

			size_t len = Tools::UINT32_MAX;
			for (size_t j = 0; j < fNode->m_conns.size(); ++j)
			{
				if (fNode->m_conns[j].nodeID == currID)
				{
					len = fNode->m_conns[j].length;
					break;
				}
			}
			assert(len != Tools::UINT32_MAX);

			destNode->m_in.insert(std::make_pair(fID, len));
		}
		if (i != 0)
		{
			LOW_ID tID = route[i - 1];

			SRC_MAP::const_iterator itrCurr = allNodes.find(currID);
			assert(itrCurr != allNodes.end());
			const TopoNode* fNode = itrCurr->second;

			size_t len = Tools::UINT32_MAX;
			for (size_t j = 0; j < fNode->m_conns.size(); ++j)
			{
				if (fNode->m_conns[j].nodeID == tID)
				{
					len = fNode->m_conns[j].length;
					break;
				}
			}
			assert(len != Tools::UINT32_MAX);

			destNode->m_out.insert(std::make_pair(tID, len));
		}
	}
}

void MidGraph::CondensedGraph::condense()
{
	setAllNodesType();

	DEST_MAP::iterator itr = m_destNodes.begin();
	for ( ; itr != m_destNodes.end(); )
	{
		CondensedTopoNode* n = itr->second;
		if (n->m_type == CondensedTopoNode::COMMON)
		{
			assert(n->m_in.size() == 1 && n->m_out.size() == 1
				|| n->m_in.size() == 2 && n->m_out.size() == 2);

			if (n->m_in.size() == 1 && n->m_out.size() == 1)
			{
				CondensedTopoNode *from = m_destNodes.find((*n->m_in.begin()).first)->second,
					*to = m_destNodes.find((*n->m_out.begin()).first)->second;
				EDGE_LENGTH newLen = (*n->m_in.begin()).second + (*n->m_out.begin()).second;
				connectTwoEdges(from, to, n->m_lowID, newLen);
			}
			else // n->m_in.size() == 2 && n->m_out.size() == 2
			{
				CondensedTopoNode *n0 = m_destNodes.find((*n->m_in.begin()).first)->second,
					*n1 = m_destNodes.find((*++n->m_in.begin()).first)->second;
				if ((*n->m_in.begin()).first == (*n->m_out.begin()).first)
				{
					connectTwoEdges(n0, n1, n->m_lowID, (*n->m_in.begin()).second + (*++n->m_out.begin()).second);
					connectTwoEdges(n1, n0, n->m_lowID, (*++n->m_in.begin()).second + (*n->m_out.begin()).second);
				}
				else
				{
					connectTwoEdges(n0, n1, n->m_lowID, (*n->m_in.begin()).second + (*n->m_out.begin()).second);
					connectTwoEdges(n1, n0, n->m_lowID, (*++n->m_in.begin()).second + (*++n->m_out.begin()).second);
				}
			}

			delete n;
			itr = m_destNodes.erase(itr);
		}
		else
			++itr;
	}
}

void MidGraph::CondensedGraph::setAllNodesID(size_t pageSize, id_type nextPage,
											 std::vector<std::vector<CondensedTopoNode*> >& dataPages)
{
	size_t offset = M_AL::DATA_COUNT_SIZE;
	std::vector<CondensedTopoNode*> dataPage;

	DEST_MAP::iterator itr = m_destNodes.begin();
	for ( ; itr != m_destNodes.end(); ++itr)
	{
		CondensedTopoNode* node = itr->second;
		size_t itemSize = 
			M_AL::SINGLE_TOPO_NODE_ID_SIZE +
			M_AL::EDGE_COUNT_SIZE + 
			M_AL::CONNECTION_ITEM_SIZE * node->m_out.size();

		if (offset + itemSize > pageSize)
		{
			dataPages.push_back(dataPage);
			dataPage.clear();
			dataPage.push_back(node);
			++nextPage;
			offset = M_AL::DATA_COUNT_SIZE + itemSize;
		}
		else
		{
			dataPage.push_back(node);
			offset += itemSize;
		}

		node->m_destID = S_AL::encodeTopoNodeID(static_cast<size_t>(nextPage), offset - itemSize);
	}

	if (!dataPage.empty())
		dataPages.push_back(dataPage);
}

void MidGraph::CondensedGraph::setEnterGridNodeIDTrans(Node* grid)
{
	std::map<SINGLE_NODE_ID, MULTI_NODE_ID>::iterator itrTrans 
		= grid->m_transEnterGridNodeIDs.begin();
	for ( ; itrTrans != grid->m_transEnterGridNodeIDs.end(); ++itrTrans)
	{		
		DEST_MAP::iterator itrDest = m_destNodes.find(itrTrans->first);
		if (itrDest != m_destNodes.end())
			itrTrans->second = itrDest->second->m_destID;
	}
}

void MidGraph::CondensedGraph::outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage,
											   const std::vector<std::vector<CondensedTopoNode*> >& dataPages)
{
	byte* data = new byte[pageSize];
	for (size_t i = 0; i < dataPages.size(); ++i)
	{
		byte* ptr = data;
		size_t dataSize = dataPages[i].size();
		memcpy(ptr, &dataSize, M_AL::DATA_COUNT_SIZE);
		ptr += M_AL::DATA_COUNT_SIZE;

		for (size_t j = 0; j < dataSize; ++j)
		{
			CondensedTopoNode* node = dataPages[i][j];
			memcpy(ptr, &node->m_lowID, M_AL::SINGLE_TOPO_NODE_ID_SIZE);
			ptr += M_AL::SINGLE_TOPO_NODE_ID_SIZE;

			size_t edgeSize = node->m_out.size();
			memcpy(ptr, &edgeSize, M_AL::EDGE_COUNT_SIZE);
			ptr += M_AL::EDGE_COUNT_SIZE;

			std::map<LOW_ID, EDGE_LENGTH>::iterator itr 
				= node->m_out.begin();
			for ( ; itr != node->m_out.end(); ++itr)
			{
				size_t connStoreID;
				if (itr->first & M_AL::TOPO_NODE_CARRY)
					connStoreID = (itr->first & M_AL::TOPO_NODE_GAP);
				else
				{
					DEST_MAP::iterator itrExist = m_destNodes.find(itr->first);
					assert(itrExist != m_destNodes.end());
					connStoreID = (itrExist->second->m_destID | M_AL::TOPO_NODE_CARRY);
				}

				memcpy(ptr, &connStoreID, M_AL::MULTI_TOPO_NODE_ID_SIZE);
				ptr += M_AL::MULTI_TOPO_NODE_ID_SIZE;
				memcpy(ptr, &itr->second, M_AL::EDGE_LENGTH_SIZE);
				ptr += M_AL::EDGE_LENGTH_SIZE;
			}
		}

		sm.storeByteArray(nextPage, pageSize, data);
		++nextPage;
	}

	delete[] data;
}

void MidGraph::CondensedGraph::setAllNodesType()
{
	DEST_MAP::iterator itr = m_destNodes.begin();
	for ( ; itr != m_destNodes.end(); ++itr)
	{
		CondensedTopoNode* n = itr->second;
		std::map<LOW_ID, EDGE_LENGTH> conn(n->m_in);
		std::map<LOW_ID, EDGE_LENGTH>::iterator itr = n->m_out.begin();
		for ( ; itr != n->m_out.end(); ++itr)
			conn.insert(*itr);

		switch (conn.size())
		{
		case 0:
			assert(0);
			break;
		case 1:
			n->m_type = CondensedTopoNode::ENDING;
			break;
		case 2:
			if (n->m_in.size() == 2 && n->m_out.size() == 2)
				n->m_type = CondensedTopoNode::COMMON;
			else
				n->m_type = CondensedTopoNode::CROSSING;
			break;
		default:
			n->m_type = CondensedTopoNode::CROSSING;
			break;
		}
	}
}

void MidGraph::CondensedGraph::connectTwoEdges(CondensedTopoNode* from, CondensedTopoNode* to, 
											   LOW_ID middleID, EDGE_LENGTH newLen)
{
	from->m_out.erase(middleID);
	from->m_out.insert(std::make_pair(to->m_lowID, newLen));
	to->m_in.erase(middleID);
	to->m_in.insert(std::make_pair(from->m_lowID, newLen));
}

//
// Internal
//

void MidGraph::expandInGrid(LOW_ID nodeID) const
{
	std::set<LOW_ID> destNodes(m_leaveGridNodeIDs);
	destNodes.erase(nodeID);

	VisitedList visitedList;
	CandidateList candidateList;

	VisitedNode* b = new VisitedNode(nodeID, 0.0, NULL);
	visitedList.push(b);
	candidateList.push(b);

	while (!candidateList.empty())
	{
		VisitedNode* opt = candidateList.top(); candidateList.pop();

		std::set<LOW_ID>::iterator itr = destNodes.find(opt->m_nodeID);
		if (itr != destNodes.end())
		{
			destNodes.erase(itr);

			std::vector<LOW_ID> path;
			opt->getShortestPath(&path);
			m_destGraph->insertRouteReversed(path, m_srcNodes);
		}
		else
		{
			SRC_MAP::const_iterator itr = m_srcNodes.find(opt->m_nodeID);
			if (itr == m_srcNodes.end())
				continue;

			const std::vector<Connection>& totConn(itr->second->m_conns);

			for (size_t i = 0; i < totConn.size(); ++i)
			{
				const Connection& ct = totConn[i];
				if ((ct.nodeID & M_AL::TOPO_NODE_CARRY) != 0)
					continue;

				PTR_VISITED pVisited = visitedList.find(ct.nodeID);
				if (visitedList.exist(pVisited))
				{
					double newFrom = opt->m_from + ct.length;
					if (newFrom < (*pVisited)->m_from)
						candidateList.update(*pVisited, newFrom, opt);
				}
				else
				{
					VisitedNode* newNode = new VisitedNode(ct.nodeID, opt->m_from + ct.length, opt);
					visitedList.push(newNode);
					candidateList.push(newNode);
				}
			}
		}

		if (destNodes.empty())
			break;
	}
}
