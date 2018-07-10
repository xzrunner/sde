#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_LAYER_NW_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_LAYER_NW_H_
#include "../NavigateBase.h"
#include "../../BasicType/BasicGMType.h"
#include "../../BasicType/Rect.h"
#include "../../NVDataPublish/Network/AdjacencyList.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			typedef NVDataPublish::Network::Connection S_CONN;

			class BindPoint;

			class SingleLayerNW : public INetWork
			{
			public:
				SingleLayerNW(IStorageManager* sm, const IPointBindLine* bind, 
					const NVDataPublish::ILookUp* table, bool bPreLoadTable);
				SingleLayerNW(IStorageManager* sm);

				virtual double getShortestDistance(size_t src, size_t dest) const;

				double computeShortestPath(BetweenTwoNodeSearchingInfo& info) const;
				virtual double computeShortestPath(SingleRouteSearchingInfo& info) const;
				virtual void computeShortestPath(MultiRoutesSearchingInfo& info) const;

				size_t queryTopoNodeID(const MapPos2D& p) const;
				void getTopoNodePos(size_t nodeID, MapPos2D* pos) const;

				size_t getConnectionSetID(size_t nodeID) const;
				void getTopoNodeConnection(size_t nodeID, std::vector<S_CONN>* conns, bool bUsedInMLSearch = false) const;

				void getStorageInfo(size_t* firstLeafPage, size_t* firstIndexPage) const {
					*firstLeafPage = m_firstLeafPage, *firstIndexPage = m_firstIndexPage;
				}
				size_t getTopoNodeSize() const;
				const Rect& getTotScope() const { return m_scope; }

				void test() const;

			private:
				void parseHeader();
				
				void parseRouteResult(const std::vector<size_t>& nodeIDs, BetweenTwoNodeSearchingInfo& info) const;
				void parseRouteResult(const std::vector<size_t>& nodeIDs, const BindPoint& s, 
					const BindPoint& e, SingleRouteSearchingInfo& info) const;
				void parseRouteResult(std::vector<std::vector<size_t> > multiRouteNodeIDs, 
					MultiRoutesSearchingInfo& info) const;

				void retrieveIndex(const MapPos2D& p, std::vector<size_t>* IDs) const;
				void retrieveLeaf(const MapPos2D& p, size_t page, size_t offset, const Rect& s, std::vector<size_t>* IDs) const;

			private:
				IStorageManager* m_buffer;

				const IPointBindLine* m_bind;

				const NVDataPublish::ILookUp* m_table;
				bool m_bPreLoadTable;

				Rect m_scope;
				bool m_rootBeLeaf;
				size_t m_firstLeafPage, m_firstIndexPage;

				friend class SLSpatialIndex;

			}; // SingleLayerNW
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_LAYER_NW_H_
