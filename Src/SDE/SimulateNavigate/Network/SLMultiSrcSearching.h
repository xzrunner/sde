#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_MULTI_SRC_SEARCHING_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_MULTI_SRC_SEARCHING_H_
#include "../NavigateBase.h"
#include "SearchingUtility.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SLSingleSrcSearching;

			class SLMultiSrcSearching
			{
			public:
				SLMultiSrcSearching(const SingleLayerNW* network);

				void queryShortestRoute(const std::vector<size_t>& nodeIDs, const MapScope& scope, 
					std::vector<std::vector<size_t> >* multiRouteNodeIDs);

			private:
				void classifyNodesByConnectionSet(const std::vector<size_t>& nodeIDs);

				void search(const MapScope& scope, std::vector<std::vector<size_t> >* multiRouteNodeIDs);
				void circleExpandInRect(const std::set<size_t>& connectNodes, const MapScope& scope, 
					std::vector<std::vector<size_t> >* multiRouteNodeIDs);
				void circleExpand(VisitedNode* n, const MapScope& scope, VisitedList& visited, CandidateList& candidate);

			private:
				const SingleLayerNW* m_network;
				SLSingleSrcSearching* m_singleSearch;

				std::map<size_t, std::set<size_t> > m_classification;

			}; // SLMultiSrcSearching
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_MULTI_SRC_SEARCHING_H_