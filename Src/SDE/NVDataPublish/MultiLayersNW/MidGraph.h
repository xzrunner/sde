#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_MIDGRAPH_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_MIDGRAPH_H_
#include "../Base.h"
#include "AdjacencyList.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			class Node;

			class CondensedGraph;
			class MidGraph
			{
			public:
				MidGraph(Node* n) : m_grid(n), m_destGraph(new CondensedGraph) {}
				~MidGraph();

				void initFromSingleNWStorage(IStorageManager& sm, const std::vector<size_t>& gridIDs);
				void initFromMultiNWStorage(IStorageManager& singleSM, IStorageManager& multiSM, 
					const std::vector<size_t>& gridIDs);

				void condenseBySearchingShortestPath() const;

				void outputCondensedGraph(IStorageManager& sm, size_t pageSize, id_type& nextPage);

			private:
				typedef size_t LOW_ID;
				typedef size_t DEST_ID;

				struct TopoNode
				{
					LOW_ID m_lowID;
					std::vector<Connection> m_conns;

					TopoNode(LOW_ID lowID) : m_lowID(lowID) {}
				}; // Node

				typedef size_t EDGE_LENGTH;
				struct CondensedTopoNode
				{
					enum CTN_TYPE
					{
						COMMON,
						ENDING,
						CROSSING
					};

					std::map<LOW_ID, EDGE_LENGTH> m_in, m_out;
						// Map ID to Length

					CTN_TYPE m_type;

					LOW_ID m_lowID;

					size_t m_destID;
						// The address in condensed Graph's storage

					CondensedTopoNode(LOW_ID lowID) : m_lowID(lowID), m_destID(0) {}

				}; // CondensedTopoNode

				typedef std::map<LOW_ID, TopoNode*>				SRC_MAP;
				typedef std::map<LOW_ID, CondensedTopoNode*>	DEST_MAP;
				class CondensedGraph
				{
				public:
					~CondensedGraph() {
						DEST_MAP::iterator itr = m_destNodes.begin();
						for ( ; itr != m_destNodes.end(); ++itr)
							delete itr->second;
					}

					void insertRouteReversed(const std::vector<LOW_ID>& route, const SRC_MAP& allNodes);

					void condense();

					void setAllNodesID(size_t pageSize, id_type nextPage, 
						std::vector<std::vector<CondensedTopoNode*> >& dataPages);

					void setEnterGridNodeIDTrans(Node* grid);

					void outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage, 
						const std::vector<std::vector<CondensedTopoNode*> >& dataPages);

				private:
					void setAllNodesType();

					void connectTwoEdges(CondensedTopoNode* from, CondensedTopoNode* to, 
						LOW_ID middleID, EDGE_LENGTH newLen);

				private:
					DEST_MAP m_destNodes;

				}; // CondensedGraph

			private:
				void expandInGrid(LOW_ID nodeID) const;

			private:
				SRC_MAP m_srcNodes;

				// todo: should be classified in connection set ID
				std::set<LOW_ID> m_leaveGridNodeIDs;

				Node* m_grid;

				CondensedGraph* m_destGraph;

			}; // MidGraph
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_MIDGRAPH_H_