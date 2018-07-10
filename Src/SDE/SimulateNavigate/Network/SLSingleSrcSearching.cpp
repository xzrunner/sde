#include "SLSingleSrcSearching.h"
#include "SingleLayerNW.h"
#include "BindPoint.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

double SLSingleSrcSearching::queryShortestRoute(const BindPoint& from, const BindPoint& to, 
												std::vector<size_t>* routeNodeID, size_t* expandNum /*= NULL*/)
{
	double ret = Tools::DOUBLE_MAX;

	if (from.m_roadID == to.m_roadID || from.m_beginID == to.m_beginID ||
		from.m_beginID == to.m_endID || from.m_endID == to.m_beginID || from.m_endID == to.m_endID)
	{
		SearchingUtility::noSearchPath(from, to, routeNodeID);
		// todo: should compute the dis in the polyline
		ret = 0;
	}
	else if (m_network->getConnectionSetID(from.m_beginID) == m_network->getConnectionSetID(to.m_beginID))
	{
		m_destInfo.initByNodePos(to, m_table, m_bPreLoadTable);
		m_visited.clear();
		m_candidate.clear();

		VisitedNode *b = new VisitedNode(from.m_beginID, from.m_beginLen, m_network, m_table, m_destInfo, NULL),
			*e = new VisitedNode(from.m_endID, from.m_endLen, m_network, m_table, m_destInfo, NULL);
		m_visited.push(b);
		m_visited.push(e);
		m_candidate.push(b);
		m_candidate.push(e);

		ret = searchPathBetweenTwoPos(routeNodeID);

		if (expandNum)
			*expandNum = m_visited.size();
	}

	return ret;
}

double SLSingleSrcSearching::queryShortestRoute(size_t from, size_t to, std::vector<size_t>* routeNodeID,
												double knownDis /*= Tools::DOUBLE_MAX*/)
{
	if (from == to)
		return 0;
	if (!m_network->getConnectionSetID(from) == m_network->getConnectionSetID(to))
		return Tools::DOUBLE_MAX;

	m_destInfo.initByNodeID(m_network, to, m_table, m_bPreLoadTable);
	m_visited.clear();
	m_candidate.clear();

	VisitedNode* b = new VisitedNode(from, 0.0, m_network, m_table, m_destInfo, NULL);
	m_visited.push(b);
	m_candidate.push(b);

	return searchPathBetweenTwoNode(routeNodeID, knownDis);
}

double SLSingleSrcSearching::queryShortestDistance(size_t from, size_t to)
{
	if (from == to)
		return 0;
	if (!m_network->getConnectionSetID(from) == m_network->getConnectionSetID(to))
		return Tools::DOUBLE_MAX;

	m_destInfo.initByNodeID(m_network, to, m_table, m_bPreLoadTable);
	m_visited.clear();
	m_candidate.clear();

	VisitedNode* b = new VisitedNode(from, 0.0, m_network, m_table, m_destInfo, NULL);
	m_visited.push(b);
	m_candidate.push(b);

	return searchDisBetweenTwoNode();
}

double SLSingleSrcSearching::searchPathBetweenTwoNode(std::vector<size_t>* pathNodeID,
													  double knownDis /*= Tools::DOUBLE_MAX*/)
{
	while (!m_candidate.empty())
	{
		VisitedNode* opt = m_candidate.top(); m_candidate.pop();
		if (m_destInfo.beSameNodeID(opt->m_nodeID))
		{
			opt->getShortestPath(pathNodeID);
			return opt->m_from;
		}
		else
		{
			if (knownDis != Tools::DOUBLE_MAX && opt->m_from + opt->m_to > knownDis)
				continue;
			expand(opt);
		}
	}

	return Tools::DOUBLE_MAX;
}

double SLSingleSrcSearching::searchPathBetweenTwoPos(std::vector<size_t>* pathNodeIDs)
{
	VisitedNode* lastResult = NULL;
	double lastDis;

	while (!m_candidate.empty())
	{
		VisitedNode* opt = m_candidate.top(); m_candidate.pop();

		if (lastResult != NULL && opt->m_from + opt->m_to > lastDis)
		{
			lastResult->getShortestPath(pathNodeIDs);
			return lastDis;
		}
		else if (m_destInfo.beSameNodeID(opt->m_nodeID))
		{
			if (lastResult != NULL)
			{
				double currDis = opt->m_from + opt->m_to;
				if (currDis < lastDis)
				{
					opt->getShortestPath(pathNodeIDs);
					return currDis;
				}
				else
				{
					lastResult->getShortestPath(pathNodeIDs);
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

double SLSingleSrcSearching::searchDisBetweenTwoNode()
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

void SLSingleSrcSearching::expand(VisitedNode* n)
{
	std::vector<S_CONN> conns;
	m_network->getTopoNodeConnection(n->m_nodeID, &conns);
	for (size_t i = 0; i < conns.size(); ++i)
	{
		const S_CONN& ct = conns.at(i);

		SearchingUtility::drawLine(m_displayInfo, ct.roadID);

		PTR_VISITED pVisited = m_visited.find(ct.nodeID);
		if (m_visited.exist(pVisited))
		{
			double newFrom = n->m_from + ct.length;
			if (newFrom < (*pVisited)->m_from)
				m_candidate.update(*pVisited, newFrom, n);
		}
		else
		{
			VisitedNode* newNode;
			if (m_bAStar)
				newNode = new VisitedNode(ct.nodeID, n->m_from + ct.length, m_network, m_table, m_destInfo, n);
			else
				newNode = new VisitedNode(ct.nodeID, n->m_from + ct.length, n);
			m_visited.push(newNode);
			m_candidate.push(newNode);
		}
	}
}
