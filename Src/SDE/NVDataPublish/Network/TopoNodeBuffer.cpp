#include "TopoNodeBuffer.h"
#include "Publish.h"
#include "AdjacencyList.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

TopoNodeBuffer::~TopoNodeBuffer()
{
	assert(m_nodes.empty());
}

void TopoNodeBuffer::addUnknownNode(const MapPos2D& dest, size_t src, uint64_t id, size_t len)
{
	UNKNOWN_NODES::iterator itr = m_nodes.find(dest);
	if (itr != m_nodes.end())
	{
		itr->second.push_back(Related(src, id, len));
	}
	else
	{
		std::vector<Related> rs;
		rs.push_back(Related(src, id, len));
		m_nodes.insert(std::make_pair(dest, rs));
	}
}

void TopoNodeBuffer::eraseUnknowNode(const MapPos2D& src, size_t nodeID)
{
	UNKNOWN_NODES::iterator itr = m_nodes.find(src);
	if (itr == m_nodes.end())
		return;

	complementTopoNodeInfo(nodeID, itr->second);

	m_nodes.erase(itr);
}

void TopoNodeBuffer::complementTopoNodeInfo()
{
	UNKNOWN_NODES::iterator itr = m_nodes.begin();
	for ( ; itr != m_nodes.end(); ++itr)
	{
		size_t unknownID = m_publish.queryTopoNodeID(itr->first);
		assert(unknownID != 0);
		complementTopoNodeInfo(unknownID, itr->second);
	}
}

void TopoNodeBuffer::complementTopoNodeInfo(size_t id, const std::vector<Related>& related)
{
	for (size_t i = 0; i < related.size(); ++i)
	{
		const Related& r = related.at(i);

		size_t page, offset;
		AdjacencyList::decodeTopoNodeID(r.m_nodeID, &page, &offset);

		byte* data;
		m_publish.readPageData(page, &data);
		byte* ptr = data + offset;

		bool bFind = false;
		ptr += AdjacencyList::COORD_SIZE * 2;

		ptr += AdjacencyList::CONNECTION_ID_SIZE;

		size_t edgeSize = 0;
		memcpy(&edgeSize, ptr, AdjacencyList::DATA_COUNT_SIZE);
		ptr += AdjacencyList::DATA_COUNT_SIZE;
		for (size_t i = 0; i < edgeSize; ++i)
		{
			size_t originalID;
			memcpy(&originalID, ptr, AdjacencyList::NODE_ID_SIZE);

			if (originalID == 0)
			{
				bFind = true;
				memcpy(ptr, &id, AdjacencyList::NODE_ID_SIZE);
				ptr += AdjacencyList::NODE_ID_SIZE;
				memcpy(ptr, &r.m_roadID, AdjacencyList::EDGE_ID_SIZE);
				ptr += AdjacencyList::EDGE_ID_SIZE;
				memcpy(ptr, &r.m_roadLength, AdjacencyList::EDGE_LENGTH_SIZE);
				ptr += AdjacencyList::EDGE_LENGTH_SIZE;
				break;
			}
			else
				ptr += AdjacencyList::CONNECTION_ITEM_SIZE;
		}

		m_publish.writePageData(page, data);

		delete[] data;
		
		if (!bFind)
		{
			assert(0);
			throw Tools::IllegalStateException(
				"TopoNodeBuffer::complementTopoNodeInfo: Can't find the node."
				);
		}
	}
}