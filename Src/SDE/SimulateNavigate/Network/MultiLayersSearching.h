#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_SEARCHING_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_SEARCHING_H_
#include "../NavigateBase.h"
#include "SearchingUtility.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			typedef NVDataPublish::ILookUp TABLE;

			class MultiLayersNW;

			class MultiLayersSearching
			{
			public:
				MultiLayersSearching(const SingleLayerNW* singleNW, const MultiLayersNW* multiNW,
					const DisProInfo& displayInfo, const TABLE* table, bool bPreLoadTable);
				MultiLayersSearching(const SingleLayerNW* singleNW, const MultiLayersNW* multiNW,
					const TABLE* table, bool bPreLoadTable);

				double queryShortestDistance(size_t from, size_t to);
				double queryShortestRoute(const BindPoint& from, const BindPoint& to, 
					std::vector<std::pair<NODE_ID, NEAREST_DIS> >* route, size_t* expandNum = NULL);

			private:
				double searchDisBetweenTwoNode();
				double searchPathBetweenTwoPos(std::vector<std::pair<NODE_ID, NEAREST_DIS> >* route);

				void expand(VisitedNode* n);

				template<class T>
				void expand(const std::vector<T>& conns, VisitedNode* n);

				size_t getExpandNodeLowID(size_t prevHighID, size_t expandFetchedID) const;
				size_t getExpandNodeHighID(size_t prevHighID, size_t expandFetchedID,
					size_t expandLowID) const;

				void drawExpandProcess(size_t src, size_t dest, size_t len) const;

			private:
				const SingleLayerNW* m_singleNW;
				const MultiLayersNW* m_multiNW;

				DisProInfo m_displayInfo;
				MapPos2D m_destPos0, m_destPos1;
				bool m_bDestSameGrid;

				const TABLE* m_table;
				bool m_bPreLoadTable;

				DestInfo m_destInfo;

				VisitedList m_visited;
				CandidateList m_candidate;

			}; // MultiLayersSearching
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_SEARCHING_H_