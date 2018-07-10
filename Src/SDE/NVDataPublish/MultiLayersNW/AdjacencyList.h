#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_ADJACENCY_LIST_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_ADJACENCY_LIST_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			struct Connection
			{
				size_t nodeID;
				size_t length;
			}; // Connection

			class AdjacencyList
			{
			public:
				static void getTopoNodePos(IStorageManager& singleSM, size_t singleID, MapPos2D* pos);
				static void getTopoNodePos(IStorageManager& singleSM, IStorageManager& multiSM, 
					size_t multiID, size_t* singleID, MapPos2D* pos);
				static void getSingleTopoNodeID(IStorageManager& multiSM, size_t multiID, size_t* singleID);

				static void fetchConnectionInfo(const byte* itemStart, std::vector<Connection>* conns);
				static size_t fetchSingleTopoNodeID(const byte* itemStart);

				static bool isNodeInRect(const MapPos2D& p, const Rect& r);

			public:
				static const size_t DATA_COUNT_SIZE				= 2;
				static const size_t SINGLE_TOPO_NODE_ID_SIZE	= 4;
				static const size_t EDGE_COUNT_SIZE				= 1;
				static const size_t MULTI_TOPO_NODE_ID_SIZE		= 4;
				static const size_t EDGE_LENGTH_SIZE			= 4;
				static const size_t CONNECTION_ITEM_SIZE		= MULTI_TOPO_NODE_ID_SIZE + EDGE_LENGTH_SIZE;

				static const size_t TOPO_NODE_GAP				= 0x7fffffff;
				static const size_t TOPO_NODE_CARRY				= 0x80000000;


			}; // AdjacencyList
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_ADJACENCY_LIST_H_