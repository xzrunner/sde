#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_SINGLE_SRC_SEARCHING_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_SINGLE_SRC_SEARCHING_H_
#include "../NavigateBase.h"
#include "SearchingUtility.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			typedef NVDataPublish::ILookUp TABLE;

			class SLSingleSrcSearching
			{
			public:
				SLSingleSrcSearching(const SingleLayerNW* network, const DisProInfo& displayInfo, const TABLE* table, bool bPreLoadTable, bool bAStar = true) 
					: m_network(network), m_displayInfo(displayInfo), m_table(table), m_bPreLoadTable(bPreLoadTable), m_bAStar(bAStar) {}
				SLSingleSrcSearching(const SingleLayerNW* network, const TABLE* table, bool bPreLoadTable, bool bAStar = true) 
					: m_network(network), m_table(table), m_bPreLoadTable(bPreLoadTable), m_bAStar(bAStar) {}

				double queryShortestRoute(const BindPoint& from, const BindPoint& to, std::vector<size_t>* routeNodeID, size_t* expandNum = NULL);
				double queryShortestRoute(size_t from, size_t to, std::vector<size_t>* routeNodeID, double knownDis = Tools::DOUBLE_MAX);

				double queryShortestDistance(size_t from, size_t to);

			private:
				double searchPathBetweenTwoNode(std::vector<size_t>* pathNodeID, double knownDis = Tools::DOUBLE_MAX);
				double searchPathBetweenTwoPos(std::vector<size_t>* pathNodeID);

				double searchDisBetweenTwoNode();

				void expand(VisitedNode* n);

			private:
				const SingleLayerNW* m_network;
				DisProInfo m_displayInfo;
				const TABLE* m_table;
				bool m_bPreLoadTable;

				DestInfo m_destInfo;

				VisitedList m_visited;
				CandidateList m_candidate;
				
				bool m_bAStar;

			}; // SLSingleSrcSearching
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SL_SINGLE_SRC_SEARCHING_H_