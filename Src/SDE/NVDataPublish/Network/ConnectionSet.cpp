#include "ConnectionSet.h"
#include "Publish.h"
#include "AdjacencyList.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

void ConnectionSet::setStrongConnectionSetID(Publish* pb, size_t endPage)
{
	size_t id = 0;

	assert(Publish::BEGIN_AL_PAGE < endPage);
	for (size_t page = Publish::BEGIN_AL_PAGE; page < endPage; ++page)
	{
		byte* data;
		pb->readPageData(page, &data);

		byte *ptr = data;
		size_t dataCount = 0;
		memcpy(&dataCount, ptr, AdjacencyList::DATA_COUNT_SIZE);
		ptr += AdjacencyList::DATA_COUNT_SIZE;

		size_t lastOffset, dataOffset;
		bool bDirty = false;
		for (size_t i = 0; i < dataCount; ++i)
		{
			dataOffset = ptr - data;
			ptr += AdjacencyList::COORD_SIZE * 2;
			lastOffset = ptr - data;

			if (bDirty)
			{
				delete[] data;
				pb->readPageData(page, &data);

				ptr = data + lastOffset;
				bDirty = false;
			}
			size_t tmpID = 0;
			memcpy(&tmpID, ptr, AdjacencyList::CONNECTION_ID_SIZE);
			ptr += AdjacencyList::CONNECTION_ID_SIZE;

			if (tmpID == AdjacencyList::CONNECTION_ID_NULL)
			{
				std::queue<ADDR> buffer;
				buffer.push(std::make_pair(page, dataOffset));
				bool bIDUsed = setConnectionID(pb, buffer, id, page, bDirty);
				if (bIDUsed)
					++id;
				assert(id < AdjacencyList::CONNECTION_ID_NULL);
			}

			size_t edgeCount = 0;
			memcpy(&edgeCount, ptr, AdjacencyList::DATA_COUNT_SIZE);
			ptr += AdjacencyList::DATA_COUNT_SIZE +	AdjacencyList::CONNECTION_ITEM_SIZE * edgeCount;
		}

		delete[] data;
	}

	//test(pb, endPage);
}

bool ConnectionSet::setConnectionID(Publish* pb, std::queue<ADDR>& buffer, size_t id,
									size_t checkPage, bool& bPageDirty)
{
	bool bSet = false;
	while (!buffer.empty())
	{
		ADDR addr = buffer.front(); buffer.pop();

		byte* data;
		pb->readPageData(addr.first, &data);
	
		byte* ptr = data + addr.second;
		ptr += AdjacencyList::COORD_SIZE * 2;
	
		size_t tmpID = 0;
		memcpy(&tmpID, ptr, AdjacencyList::CONNECTION_ID_SIZE);
		if (tmpID == AdjacencyList::CONNECTION_ID_NULL)
		{
			bSet = true;

			if (checkPage == addr.first)
				bPageDirty = true;

			memcpy(ptr, &id, AdjacencyList::CONNECTION_ID_SIZE);
			ptr += AdjacencyList::CONNECTION_ID_SIZE;

			pb->writePageData(addr.first, data);

			size_t edgeCount = 0;
			memcpy(&edgeCount, ptr, AdjacencyList::DATA_COUNT_SIZE);
			ptr += AdjacencyList::DATA_COUNT_SIZE;
			for (size_t j = 0; j < edgeCount; ++j)
			{
				size_t nodeID = 0;
				memcpy(&nodeID, ptr, AdjacencyList::NODE_ID_SIZE);
				ptr += AdjacencyList::CONNECTION_ITEM_SIZE;

				size_t page, offset;
				AdjacencyList::decodeTopoNodeID(nodeID, &page, &offset);
				buffer.push(std::make_pair(page, offset));
			}
		}

		delete[] data;
	}

	return bSet;
}

void ConnectionSet::test(Publish* pb, size_t endPage)
{
	std::map<size_t, std::vector<MapPos2D> > mapID2Pos;

	assert(Publish::BEGIN_AL_PAGE < endPage);
	for (size_t page = Publish::BEGIN_AL_PAGE; page < endPage; ++page)
	{
		byte* data;
		pb->readPageData(page, &data);

		byte *ptr = data;
		size_t dataCount = 0;
		memcpy(&dataCount, ptr, AdjacencyList::DATA_COUNT_SIZE);
		ptr += AdjacencyList::DATA_COUNT_SIZE;

		for (size_t i = 0; i < dataCount; ++i)
		{
			int cx, cy;
			memcpy(&cx, ptr, AdjacencyList::COORD_SIZE);
			ptr += AdjacencyList::COORD_SIZE;
			memcpy(&cy, ptr, AdjacencyList::COORD_SIZE);
			ptr += AdjacencyList::COORD_SIZE;

			size_t id = 0;
			memcpy(&id, ptr, AdjacencyList::CONNECTION_ID_SIZE);
			ptr += AdjacencyList::CONNECTION_ID_SIZE;

			MapPos2D pos;
			pos.d0 = cx;
			pos.d1 = cy;

			std::map<size_t, std::vector<MapPos2D> >::iterator itr = mapID2Pos.find(id);
			if (itr == mapID2Pos.end())
			{
				std::vector<MapPos2D> tmp;
				tmp.push_back(pos);
				mapID2Pos.insert(std::make_pair(id, tmp));
			}
			else
				itr->second.push_back(pos);

			size_t edgeCount = 0;
			memcpy(&edgeCount, ptr, AdjacencyList::DATA_COUNT_SIZE);
			ptr += AdjacencyList::DATA_COUNT_SIZE;

			ptr += AdjacencyList::CONNECTION_ITEM_SIZE * edgeCount;
		}

		delete[] data;
	}
}