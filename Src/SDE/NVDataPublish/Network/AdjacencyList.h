#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_ADJACENCY_LIST_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_ADJACENCY_LIST_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class Leaf;
			class TopoNodeBuffer;
			class Publish;

			struct Connection
			{
				size_t nodeID;
				uint64_t roadID;
				size_t length;
			}; // Connection

			class AdjacencyList
			{
			public:
				AdjacencyList(Leaf* leaf, TopoNodeBuffer& buffer, size_t& nextPage);
				~AdjacencyList();

				void createTopoData(Publish& pb, size_t& nodeCount);

				static size_t computeDataOffset(const MapPos2D& p, byte* data);

				static size_t fetchNodeCount(const byte* pageStart);
				static void fetchAllNodesPos(const byte* pageStart, std::vector<MapPos2D>& pos);

				static size_t fetchConnectionSetID(const byte* itemStart);
				static void fetchCoordinate(const byte* itemStart, MapPos2D* pos);
				static void fetchConnectionInfo(const byte* itemStart, std::vector<Connection>* conns);

				static size_t encodeTopoNodeID(size_t page, size_t offset);
				static void decodeTopoNodeID(size_t id, size_t* page, size_t* offset);

				static size_t encodeTopoEdgeLength(size_t original);
				static size_t decodeTopoEdgeLength(size_t storage);

			public:
				// Page
				static const size_t COORD_SIZE			= 4;
				static const size_t CONNECTION_ID_SIZE	= 2;
				static const size_t DATA_COUNT_SIZE		= 1;
				static const size_t NODE_ID_SIZE		= 4;
				static const size_t EDGE_ID_SIZE		= 8;
				static const size_t EDGE_LENGTH_SIZE	= 2;

				static const size_t CONNECTION_ITEM_SIZE= NODE_ID_SIZE + EDGE_ID_SIZE + EDGE_LENGTH_SIZE;

				static const size_t EDGE_LENGTH_CARRY			= 0x8000;
				static const size_t EDGE_LENGTH_ENDLESS			= 0xffff;
				static const size_t EDGE_LENGTH_CONDENSE_BITS	= 10;

				static const size_t CONNECTION_ID_NULL	= 0xffff;

				// Node ID
				static const size_t TOPO_NODE_GAP		= 0x7fffffff;
				static const size_t TOPO_NODE_CARRY		= 0x80000000;
				static const size_t GRID_SIZE_BIT		= static_cast<size_t>(8 * 2.5);
				static const size_t OFFSET_SIZE_BIT		= static_cast<size_t>(8 * 1.5);

				static const uint64_t MAX_DATA_COUNT	= 0x00000000000000ff;

			private:
				class TopoNode
				{
				public:
					TopoNode(const MapPos2D& p, bool bInScope = true) 
						: m_pos(p), m_id(0), m_bInScope(bInScope) {}

					size_t getDataSize() const;

				public:
					class Related
					{
					public:
						Related(const MapPos2D& p, uint64_t id, size_t len)
							: m_pos(p), m_roadID(id), m_roadLength(len) {}

					public:
						MapPos2D m_pos;
						uint64_t m_roadID;
						size_t m_roadLength;

					}; // Related

				public:
					MapPos2D m_pos;
					std::vector<Related> m_relation;

					bool m_bInScope;

					size_t m_id;

				}; // TopoNode

				class TopoNodeHorCmp
				{
				public:
					bool operator () (const TopoNode* lhs, const TopoNode* rhs) const {
						return lhs->m_pos.d0 < rhs->m_pos.d0
							|| lhs->m_pos.d0 == rhs->m_pos.d0 && lhs->m_pos.d1 < rhs->m_pos.d1;
					}
				}; // TopoNodeHorCmp

				class TopoNodeVerCmp
				{
				public:
					bool operator () (const TopoNode* lhs, const TopoNode* rhs) const {
						return lhs->m_pos.d1 < rhs->m_pos.d1
							|| lhs->m_pos.d1 == rhs->m_pos.d1 && lhs->m_pos.d0 < rhs->m_pos.d0;
					}
				}; // TopoNodeVerCmp

				class OnePageData
				{
				public:
					void outPutToPage(
						byte* page, 
						const std::set<TopoNode*, TopoNodeHorCmp>& totNodes,
						TopoNodeBuffer& topoNodeBuffer
						) const;

				public:
					std::vector<const TopoNode*> m_topoNodes;

				}; // OnePageData

				class TopoNodeLoader
				{
				public:
					static void loadTotal(
						const std::set<TopoNode*, TopoNodeHorCmp>& nodes,
						std::vector<std::vector<OnePageData*> >& data, 
						Leaf* leaf,
						size_t nextPage,
						TopoNodeBuffer& topoNodeBuffer,
						size_t verSize, 
						size_t pageSize
						);

				private:
					static void loadVertical(
						const std::set<TopoNode*, TopoNodeVerCmp>& nodes,
						size_t pageSize,
						std::vector<double>& horBound,
						std::vector<OnePageData*>& horPages,
						size_t& nextPage,
						TopoNodeBuffer& topoNodeBuffer
						);

				}; // TopoNodeLoader

			private:
				void buildTopoStruct(Publish& pb);

				void partitionScopeToPage(std::vector<std::vector<OnePageData*> >& totPageData, size_t pageSize);

				void outputTopoStruct(const std::vector<std::vector<OnePageData*> >& totPageData, Publish& pb) const;

			private:
				void insertEdge(const MapPos2D& s, const MapPos2D& e, uint64_t roadID, size_t roadLength,
					bool bEndIn, const Publish& pb);

				size_t getTotDataSize() const;

			private:
				Leaf* m_leaf;
				TopoNodeBuffer& m_buffer;
				size_t& m_nextPage;

				std::set<TopoNode*, TopoNodeHorCmp> m_nodes;

			}; // AdjacencyList
		}
	}	
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_ADJACENCY_LIST_H_
