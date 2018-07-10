#include "AdjacencyList.h"
#include "../Network/AdjacencyList.h"
#include "../../BasicType/Rect.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW;

typedef NVDataPublish::Network::AdjacencyList S_AL;

void AdjacencyList::getTopoNodePos(IStorageManager& singleSM, size_t singleID, MapPos2D* pos)
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(singleID, &page, &offset);

	byte* data;
	size_t len;
	singleSM.loadByteArray(page, len, &data);

	byte* ptr = data + offset;
	S_AL::fetchCoordinate(ptr, pos);

	delete[] data;
}

void AdjacencyList::getTopoNodePos(IStorageManager& singleSM, IStorageManager& multiSM, 
								   size_t multiID, size_t* singleID, MapPos2D* pos)
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(multiID, &page, &offset);

	byte* data;
	size_t len;
	multiSM.loadByteArray(page, len, &data);
	byte* ptr = data + offset;

	*singleID = 0;
	memcpy(singleID, ptr, SINGLE_TOPO_NODE_ID_SIZE);
	delete[] data;

	return getTopoNodePos(singleSM, *singleID, pos);
}

void AdjacencyList::getSingleTopoNodeID(IStorageManager& multiSM, size_t multiID, size_t* singleID)
{
	size_t page, offset;
	S_AL::decodeTopoNodeID(multiID, &page, &offset);

	byte* data;
	size_t len;
	multiSM.loadByteArray(page, len, &data);
	byte* ptr = data + offset;

	*singleID = 0;
	memcpy(singleID, ptr, SINGLE_TOPO_NODE_ID_SIZE);
	delete[] data;
}

void AdjacencyList::fetchConnectionInfo(const byte* itemStart, std::vector<Connection>* conns)
{
	itemStart += SINGLE_TOPO_NODE_ID_SIZE;

	size_t cEdge = 0;
	memcpy(&cEdge, itemStart, EDGE_COUNT_SIZE);
	itemStart += EDGE_COUNT_SIZE;

	conns->reserve(cEdge);
	for (size_t i = 0; i < cEdge; ++i)
	{
		Connection ct;
		memcpy(&ct.nodeID, itemStart, MULTI_TOPO_NODE_ID_SIZE);
		itemStart += MULTI_TOPO_NODE_ID_SIZE;
		memcpy(&ct.length, itemStart, EDGE_LENGTH_SIZE);
		itemStart += EDGE_LENGTH_SIZE;

		conns->push_back(ct);
	}
}

size_t AdjacencyList::fetchSingleTopoNodeID(const byte* itemStart)
{
	size_t ret = 0;
	memcpy(&ret, itemStart, SINGLE_TOPO_NODE_ID_SIZE);
	return ret;
}

bool AdjacencyList::isNodeInRect(const MapPos2D& p, const Rect& r)
{
	return p.d0 >= r.m_pLow[0] && p.d0 < r.m_pHigh[0] 
		&& p.d1 >= r.m_pLow[1] && p.d1 < r.m_pHigh[1];
}