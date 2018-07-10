#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_NW_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_NW_H_
#include "../NavigateBase.h"
#include "../../NVDataPublish/MultiLayersNW/AdjacencyList.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			typedef size_t							NODE_ID;
			typedef size_t							NEAREST_DIS;
			typedef NVDataPublish::MLNW::Connection M_CONN;

			class BindPoint;
			class SingleLayerNW;

			class MultiLayersNW : public INetWork
			{
			public:
				MultiLayersNW(IStorageManager* sm, const SingleLayerNW* singleNW, 
					const IPointBindLine* bind, const NVDataPublish::ILookUp* table, bool bPreLoadTable);
				virtual ~MultiLayersNW() { delete m_singleNW; }

				virtual double getShortestDistance(size_t src, size_t dest) const;
				virtual double computeShortestPath(SingleRouteSearchingInfo& info) const;
				virtual void computeShortestPath(MultiRoutesSearchingInfo& info) const;

				void getTopoNodeConnection(size_t nodeID, std::vector<M_CONN>* conns) const;
				size_t getTopoNodeLowID(size_t nodeID) const;

				size_t getCrossGridHighID(size_t expandLowID, const MapPos2D& dest0, 
					const MapPos2D& dest1, bool bDestSameGrid) const;

				bool isTwoPointInSameLeaf(const MapPos2D& p0, const MapPos2D& p1) const {
					return m_spatialIndex.isTwoPointInSameLeaf(p0, p1);
				}

				// for test
				void getAllGridScopeByPostorderTraversal(std::vector<Rect>& rects) const {
					m_spatialIndex.getAllScopeByPostorderTraversal(rects);
				}

			private:
				class SpatialIndex
				{
				public:
					SpatialIndex() { m_scope.makeInfinite(); }
					~SpatialIndex() { delete m_root; }

					void initByParseStorage(IStorageManager* storage, size_t pageSize, 
						size_t indexPageCount);

					size_t getHighestLayerGridBetweenPos(const MapPos2D& locate, 
						const MapPos2D& except) const;

					bool isTwoPointInSameLeaf(const MapPos2D& p0, const MapPos2D& p1) const;

					// for test
					void getAllScopeByPostorderTraversal(std::vector<Rect>& rects) const;

				private:
					class Grid
					{
					public:
						Grid() {
							for (size_t i = 0; i < 4; ++i)
								m_child[i] = NULL;
						}
						~Grid() {
							for (size_t i = 0; i < 4; ++i)
								delete m_child[i];
						}

						bool isLeaf() const { return m_child[0] == NULL; }

						// for test
						void postorderTraversalFetchScope(const Rect& myScope, 
							std::vector<Rect>& rects);

					private:
						Grid* m_child[4];

						friend class MultiLayersNW;

					}; // Grid

				private:
					Rect m_scope;
					Grid* m_root;

				}; // SpatialIndex

				class ComeInNodesLookUpTable
				{
				public:
					void initByParseStorage(IStorageManager* storage, size_t pageSize, 
						size_t tablePageStart);

					size_t mapSingleIDAndLayerToMultiID(size_t singleID, size_t layer) const;

				private:
					size_t queryFromIndex1(size_t singleID) const;
					size_t queryFromIndex2(size_t pageID, size_t singleID) const;
					size_t queryFromData(size_t addr, size_t layer) const;

				private:
					IStorageManager* m_storage;

					std::vector<size_t> m_index1;

					size_t m_pageStart;

				}; // ComeInNodesLookUpTable

			private:
				void parseHeader(size_t& pageSize, size_t& indexPageCount, size_t& tablePageStart);

				void parseRouteResult(const std::vector<std::pair<NODE_ID, NEAREST_DIS> >& route, 
					const BindPoint& s, const BindPoint& e, SingleRouteSearchingInfo& info) const;

			private:
				IStorageManager* m_buffer;

				const IPointBindLine* m_bind;

				const NVDataPublish::ILookUp* m_table;
				bool m_bPreLoadTable;

				const SingleLayerNW* m_singleNW;

				SpatialIndex m_spatialIndex;

				ComeInNodesLookUpTable m_comeInNodesLookUpTable;

			}; // MultiLayersNW
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_MULTI_LAYERS_NW_H_