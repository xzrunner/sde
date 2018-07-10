#include "SLSpatialIndex.h"
#include "SingleLayerNW.h"
#include "../../NVDataPublish/Network/Leaf.h"
#include "../../NVDataPublish/Network/Publish.h"
#include "../../NVDataPublish/Network/AdjacencyList.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

typedef NVDataPublish::Network::AdjacencyList	AL;
typedef NVDataPublish::Network::Publish			PB;
typedef NVDataPublish::Network::Node			Node;

//
// class SNode
//

SNode::SNode(const Rect& scope, size_t count, bool bLeaf)
	: m_scope(scope), m_topoNodeCount(count), m_bLeaf(bLeaf)
{
}

//
// class SLeaf
//

SLeaf::SLeaf(const Rect& scope, size_t count)
	: SNode(scope, count, true)
{
}

//
// class SIndex
//

SIndex::SIndex(const Rect& scope, size_t count)
	: SNode(scope, count, false)
{
}

SIndex::~SIndex()
{
	for (size_t i = 0; i < 4; ++i)
		delete m_child[i];
}

//
// class SLSpatialIndex
//

SLSpatialIndex::SLSpatialIndex(IStorageManager& sm)
	: m_network(new SingleLayerNW(&sm)), m_scope(m_network->m_scope)
{
	build();
}

SLSpatialIndex::~SLSpatialIndex()
{
	delete m_root;
	delete m_network;
}

void SLSpatialIndex::build()
{
	if (m_network->m_rootBeLeaf)
	{
		byte* data;
		size_t len;
		m_network->m_buffer->loadByteArray(m_network->m_firstLeafPage, len, &data);

		SLeaf* l = new SLeaf(m_network->m_scope, 0);
		NVDataPublish::Network::Leaf::getAllAdjacencyListIDs(
			data + PB::LEAF_COUNT_SIZE, l->m_startALPage, l->m_pageCount
			);
		delete[] data;

		l->m_topoNodeCount = 0;
		for (size_t i = 0; i < l->m_pageCount; ++i)
		{
			m_network->m_buffer->loadByteArray(l->m_startALPage + i, len, &data);
			l->m_topoNodeCount += AL::fetchNodeCount(data);
			delete[] data;
		}

		m_root = l;
	}
	else
	{
		m_root = new SIndex(m_network->m_scope, 0);
		std::queue<std::pair<SIndex*, ADDR> > queIndexes;
		queIndexes.push(std::make_pair(dynamic_cast<SIndex*>(m_root), std::make_pair(m_network->m_firstIndexPage, 0)));

		while (!queIndexes.empty())
		{
			std::pair<SIndex*, ADDR> item = queIndexes.front(); queIndexes.pop();

			byte* data;
			size_t len;
			m_network->m_buffer->loadByteArray(item.second.first, len, &data);
			byte* ptr = data + item.second.second;

			Rect childScope(item.first->m_scope);
			for (size_t i = 0; i < 4; ++i)
			{
				Tools::SpatialMath::getRectChildGrid(item.first->m_scope, i, childScope);

				size_t id, count;
				memcpy(&id, ptr, Node::PM_NODE_ID_SIZE);
				ptr += Node::PM_NODE_ID_SIZE;
				memcpy(&count, ptr, Node::PM_NODE_COUNT_SIZE);
				ptr += Node::PM_NODE_COUNT_SIZE;

				ADDR addr;
				AL::decodeTopoNodeID(id, &addr.first, &addr.second);

				if (addr.first < m_network->m_firstIndexPage)
				{
					if (addr.first == 0 && addr.second == 0)
					{
						SLeaf* l = new SLeaf(m_network->m_scope, 0);
						l->m_startALPage = 0;
						l->m_pageCount = 0;
						item.first->m_child[i] = l;
					}
					else
					{
						byte* data;
						size_t len;
						m_network->m_buffer->loadByteArray(addr.first, len, &data);
						SLeaf* l = new SLeaf(childScope, count);
						NVDataPublish::Network::Leaf::getAllAdjacencyListIDs(
							data + addr.second, l->m_startALPage, l->m_pageCount
							);
						item.first->m_child[i] = l;
						delete[] data;
					}
				}
				else
				{
					SIndex* tmp = new SIndex(childScope, count);
					item.first->m_child[i] = tmp;
					queIndexes.push(std::make_pair(tmp, addr));
				}
			}

			delete[] data;
		}

		for (size_t i = 0; i < 4; ++i)
			m_root->m_topoNodeCount += dynamic_cast<SIndex*>(m_root)->m_child[i]->m_topoNodeCount;
	}
}