#include "SLMultiSrcSearching.h"
#include "SingleLayerNW.h"
#include "SLSingleSrcSearching.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

typedef std::map<size_t, std::set<size_t> >::iterator SET_ITR;

SLMultiSrcSearching::SLMultiSrcSearching(const SingleLayerNW* network)
	: m_network(network), m_singleSearch(new SLSingleSrcSearching(network, NULL, false))
{
}

void SLMultiSrcSearching::queryShortestRoute(const std::vector<size_t>& nodeIDs, const MapScope& scope, 
											 std::vector<std::vector<size_t> >* multiRouteNodeIDs)
{
	classifyNodesByConnectionSet(nodeIDs);

	search(scope, multiRouteNodeIDs);
}

void SLMultiSrcSearching::classifyNodesByConnectionSet(const std::vector<size_t>& nodeIDs)
{
	for (size_t i = 0; i < nodeIDs.size(); ++i)
	{
		size_t id = nodeIDs[i];
		size_t setID = m_network->getConnectionSetID(id);

		SET_ITR itr = m_classification.find(setID);
		if (itr == m_classification.end())
		{
			std::set<size_t> tmp;
			tmp.insert(id);
			m_classification.insert(std::make_pair(setID, tmp));
		}
		else
			itr->second.insert(id);
	}
}

void SLMultiSrcSearching::search(const MapScope& scope, std::vector<std::vector<size_t> >* multiRouteNodeIDs)
{
	std::vector<size_t> nodeIDs;

	SET_ITR itr = m_classification.begin();
	for ( ; itr != m_classification.end(); ++itr)
	{
		if (itr->second.size() < 2)
			continue;
		else if (itr->second.size() == 2)
		{
			size_t n0 = *itr->second.begin(), 
				n1 = *++itr->second.begin();

			nodeIDs.clear();
			m_singleSearch->queryShortestRoute(n0, n1, &nodeIDs);
			multiRouteNodeIDs->push_back(nodeIDs);
			nodeIDs.clear();
			m_singleSearch->queryShortestRoute(n1, n0, &nodeIDs);
			multiRouteNodeIDs->push_back(nodeIDs);
		}
		else
			circleExpandInRect(itr->second, scope, multiRouteNodeIDs);
	}
}

void SLMultiSrcSearching::circleExpandInRect(const std::set<size_t>& connectNodes, const MapScope& scope, 
											 std::vector<std::vector<size_t> >* multiRouteNodeIDs)
{
	std::set<size_t>::const_iterator itr = connectNodes.begin();
	for ( ; itr != connectNodes.end(); ++itr)
	{
		size_t curr = *itr;
		std::set<size_t> others(connectNodes);
		others.erase(curr);

		VisitedList visitedList;
		CandidateList candidateList;

		VisitedNode* b = new VisitedNode(curr, 0.0, NULL);
		visitedList.push(b);
		candidateList.push(b);

		while (!candidateList.empty())
		{
			VisitedNode* opt = candidateList.top(); candidateList.pop();
			
			std::set<size_t>::iterator itr = others.find(opt->m_nodeID);
			if (itr != others.end())
			{
				others.erase(itr);
				std::vector<size_t> path;
				opt->getShortestPath(&path);
				multiRouteNodeIDs->push_back(path);
			}
			else
				circleExpand(opt, scope, visitedList, candidateList);

			if (others.empty())
				break;
		}
	}
}

void SLMultiSrcSearching::circleExpand(VisitedNode* n, const MapScope& scope, VisitedList& visited, CandidateList& candidate)
{
	std::vector<S_CONN> conns;
	m_network->getTopoNodeConnection(n->m_nodeID, &conns);
	for (size_t i = 0; i < conns.size(); ++i)
	{
		const S_CONN& ct = conns.at(i);

		PTR_VISITED pVisited = visited.find(ct.nodeID);
		if (visited.exist(pVisited))
		{
			double newFrom = n->m_from + ct.length;
			if (newFrom < (*pVisited)->m_from)
				candidate.update(*pVisited, newFrom, n);
		}
		else
		{
			MapPos2D pos;
			m_network->getTopoNodePos(ct.nodeID, &pos);
			if (Tools::SpatialMath::isPointInRect(pos, scope))
			{
				VisitedNode* newNode = new VisitedNode(ct.nodeID, n->m_from + ct.length, n);
				visited.push(newNode);
				candidate.push(newNode);
			}
		}
	}
}