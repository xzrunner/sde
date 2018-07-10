#include "MultiLayersSearching.h"
#include "SingleLayerNW.h"
#include "MultiLayersNW.h"
#include "BindPoint.h"
#include "../../NVDataPublish/MultiLayersNW/AdjacencyList.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

typedef NVDataPublish::MLNW::AdjacencyList	M_AL;

MultiLayersSearching::MultiLayersSearching(const SingleLayerNW* singleNW, const MultiLayersNW* multiNW, 
										   const DisProInfo& displayInfo, const TABLE* table, bool bPreLoadTable)
	: m_singleNW(singleNW), m_multiNW(multiNW), m_displayInfo(displayInfo), m_table(table), m_bPreLoadTable(bPreLoadTable)
{
}

MultiLayersSearching::MultiLayersSearching(const SingleLayerNW* singleNW, const MultiLayersNW* multiNW, 
										   const TABLE* table, bool bPreLoadTable)
										   : m_singleNW(singleNW), m_multiNW(multiNW), m_table(table), m_bPreLoadTable(bPreLoadTable)
{
}

double MultiLayersSearching::queryShortestDistance(size_t from, size_t to)
{
	if (from == to)
		return 0;
	if (!m_singleNW->getConnectionSetID(from) == m_singleNW->getConnectionSetID(to))
		return Tools::DOUBLE_MAX;

	m_destInfo.initByNodeID(m_singleNW, to, m_table, m_bPreLoadTable);
	MapPos2D destPos;
	m_singleNW->getTopoNodePos(to, &destPos);
	m_destPos0 = destPos;
	m_destPos1 = destPos;
	m_bDestSameGrid = true;

	m_visited.clear();
	m_candidate.clear();

	VisitedNode* b = new VisitedNode(from, 0.0, m_singleNW, m_table, m_destInfo, NULL);
	m_visited.push(b);
	m_candidate.push(b);

	return searchDisBetweenTwoNode();

}

double MultiLayersSearching::queryShortestRoute(const BindPoint& from, const BindPoint& to, 
												std::vector<std::pair<NODE_ID, NEAREST_DIS> >* route,
												size_t* expandNum /*= NULL*/)
{
	double ret = Tools::DOUBLE_MAX;

	if (from.m_roadID == to.m_roadID || from.m_beginID == to.m_beginID ||
		from.m_beginID == to.m_endID || from.m_endID == to.m_beginID || from.m_endID == to.m_endID)
	{
//		SearchingUtility::noSearchPath(from, to, routeNodeID);
		// todo: should compute the dis in the polyline
		ret = 0;
	}
	else if (m_singleNW->getConnectionSetID(from.m_beginID) == m_singleNW->getConnectionSetID(to.m_beginID))
	{
		m_destInfo.initByNodePos(to, m_table, m_bPreLoadTable);
		m_destPos0 = to.m_beginPos;
		m_destPos1 = to.m_endPos;
		m_bDestSameGrid = m_multiNW->isTwoPointInSameLeaf(m_destPos0, m_destPos1);

		m_visited.clear();
		m_candidate.clear();

		VisitedNode *b = new VisitedNode(from.m_beginID, from.m_beginLen, m_singleNW, m_table, m_destInfo, NULL),
			*e = new VisitedNode(from.m_endID, from.m_endLen, m_singleNW, m_table, m_destInfo, NULL);
		m_visited.push(b);
		m_visited.push(e);
		m_candidate.push(b);
		m_candidate.push(e);

		ret = searchPathBetweenTwoPos(route);

		if (expandNum)
			*expandNum = m_visited.size();
	}

	return ret;
}

double MultiLayersSearching::searchDisBetweenTwoNode()
{
	while (!m_candidate.empty())
	{
		VisitedNode* opt = m_candidate.top(); m_candidate.pop();
		if (m_destInfo.beSameNodeID(opt->m_nodeID))
			return opt->m_from;
		else
			expand(opt);
	}

	return Tools::DOUBLE_MAX;
}

double MultiLayersSearching::searchPathBetweenTwoPos(std::vector<std::pair<NODE_ID, NEAREST_DIS> >* route)
{
	VisitedNode* lastResult = NULL;
	double lastDis;

	while (!m_candidate.empty())
	{
		VisitedNode* opt = m_candidate.top(); m_candidate.pop();

		if (lastResult != NULL && opt->m_from + opt->m_to > lastDis)
		{
			lastResult->getShortestPath(route);
			return lastDis;
		}
		else if (m_destInfo.beSameNodeID(opt->m_nodeID))
		{
			if (lastResult != NULL)
			{
				double currDis = opt->m_from + opt->m_to;
				if (currDis < lastDis)
				{
					opt->getShortestPath(route);
					return currDis;
				}
				else
				{
					lastResult->getShortestPath(route);
					return lastDis;
				}
			}
			else
			{
				lastResult = opt;
				lastDis = lastResult->m_from + lastResult->m_to;
				expand(opt);
			}
		}
		else
			expand(opt);
	}

	return Tools::DOUBLE_MAX;
}

void MultiLayersSearching::expand(VisitedNode* n)
{
	if (n->m_highID == 0)
	{
		std::vector<S_CONN> conns;
		m_singleNW->getTopoNodeConnection(n->m_nodeID, &conns, true);
		expand(conns, n);
	}
	else
	{
		std::vector<M_CONN> conns;
		m_multiNW->getTopoNodeConnection(n->m_highID, &conns);
		expand(conns, n);
	}
}

template<class T>
void MultiLayersSearching::expand(const std::vector<T>& conns, VisitedNode* n)
{
	for (size_t i = 0; i < conns.size(); ++i)
	{
		const T& ct = conns[i];

		size_t expandLowID = getExpandNodeLowID(n->m_highID, ct.nodeID);

		PTR_VISITED pVisited = m_visited.find(expandLowID);
		if (m_visited.exist(pVisited))
		{
			double newFrom = n->m_from + ct.length;
			if (newFrom < (*pVisited)->m_from)
				m_candidate.update(*pVisited, newFrom, n);
		}
		else
		{
			drawExpandProcess(n->m_nodeID, expandLowID, ct.length);

			size_t expandHighID = getExpandNodeHighID(n->m_highID, ct.nodeID, expandLowID);

			VisitedNode* newNode = new VisitedNode(expandLowID, expandHighID, 
				n->m_from + ct.length, m_singleNW, m_table, m_destInfo, n);
			m_visited.push(newNode);
			m_candidate.push(newNode);
		}
	}
}

size_t MultiLayersSearching::getExpandNodeLowID(size_t prevHighID, size_t expandFetchedID) const
{
	if (prevHighID == 0)
		return expandFetchedID & M_AL::TOPO_NODE_GAP;
	else
	{
		if (expandFetchedID & M_AL::TOPO_NODE_CARRY)
			return m_multiNW->getTopoNodeLowID(expandFetchedID);
		else
			return expandFetchedID;
	}
}

size_t MultiLayersSearching::getExpandNodeHighID(size_t prevHighID, size_t expandFetchedID,
												 size_t expandLowID) const
{
	if (prevHighID != 0 && (expandFetchedID & M_AL::TOPO_NODE_CARRY))
		return expandFetchedID;
	else if (prevHighID == 0 && (expandFetchedID & M_AL::TOPO_NODE_CARRY) == 0)
		return 0;
	else
		return m_multiNW->getCrossGridHighID(expandLowID, m_destPos0, m_destPos1, m_bDestSameGrid);
}

void MultiLayersSearching::drawExpandProcess(size_t src, size_t dest, size_t len) const
{
	if (m_displayInfo.isValid())
	{
		SimulateNavigate::BetweenTwoNodeSearchingInfo part(src, dest, len);
		m_singleNW->computeShortestPath(part);

		std::vector<uint64_t>::iterator itr = part.m_roadIDs.begin();
		for ( ; itr != part.m_roadIDs.end(); ++itr)
			SearchingUtility::drawLine(m_displayInfo, *itr);
	}
}