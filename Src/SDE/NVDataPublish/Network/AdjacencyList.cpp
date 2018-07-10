#include "AdjacencyList.h"
#include "Publish.h"
#include "Leaf.h"
#include "MapPartition.h"
#include "TopoNodeBuffer.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

AdjacencyList::AdjacencyList(Leaf* leaf, TopoNodeBuffer& buffer, size_t& nextPage) 
	: m_leaf(leaf), m_buffer(buffer), m_nextPage(nextPage)
{
}

AdjacencyList::~AdjacencyList()
{
	std::set<TopoNode*, TopoNodeHorCmp>::iterator itr = m_nodes.begin();
	for ( ; itr != m_nodes.end(); ++itr)
		delete *itr;
}

void AdjacencyList::createTopoData(Publish& pb, size_t& nodeCount)
{
	buildTopoStruct(pb);

	if (m_nodes.empty())
	{
		m_leaf->m_bEmpty = true;
	}
	else
	{
		m_leaf->m_bEmpty = false;
		std::vector<std::vector<OnePageData*> > totPageData;
		partitionScopeToPage(totPageData, pb.m_pageSize);
		outputTopoStruct(totPageData, pb);

		for (size_t i = 0; i < totPageData.size(); ++i)
		{
			for (size_t j = 0; j < totPageData[i].size(); ++j)
			{
				nodeCount += totPageData[i][j]->m_topoNodes.size();
				delete totPageData[i][j];
			}
		}
	}
}

size_t AdjacencyList::computeDataOffset(const MapPos2D& p, byte* data)
{
	byte* start = data;
	size_t dataCount = 0;
	memcpy(&dataCount, data, DATA_COUNT_SIZE);
	data += DATA_COUNT_SIZE;

	int x, y;
	x = static_cast<int>(p.d0 + 0.5);
	y = static_cast<int>(p.d1 + 0.5);

	for (size_t i = 0; i < dataCount; ++i)
	{
		int cx, cy;
		memcpy(&cx, data, COORD_SIZE);
		data += COORD_SIZE;
		memcpy(&cy, data, COORD_SIZE);
		data += COORD_SIZE;

		if (cx == x && cy == y)
		{
			return data - start - COORD_SIZE * 2;
		}
		else
		{
			data += CONNECTION_ID_SIZE;
			size_t edgeCount = 0;
			memcpy(&edgeCount, data, DATA_COUNT_SIZE);
			data += DATA_COUNT_SIZE;
			data += edgeCount * CONNECTION_ITEM_SIZE;
		}
	}

	return 0;
}

size_t AdjacencyList::fetchNodeCount(const byte* pageStart)
{
	size_t c = 0;
	memcpy(&c, pageStart, DATA_COUNT_SIZE);
	return c;
}

void AdjacencyList::fetchAllNodesPos(const byte* pageStart, std::vector<MapPos2D>& pos)
{
	const byte* ptr = pageStart;
	size_t dataCount = 0;
	memcpy(&dataCount, ptr, DATA_COUNT_SIZE);
	ptr += DATA_COUNT_SIZE;

	for (size_t i = 0; i < dataCount; ++i)
	{
		int cx, cy;
		memcpy(&cx, ptr, COORD_SIZE);
		ptr += COORD_SIZE;
		memcpy(&cy, ptr, COORD_SIZE);
		ptr += COORD_SIZE;
		pos.push_back(MapPos2D(cx, cy));

		ptr += CONNECTION_ID_SIZE;
		size_t edgeCount = 0;
		memcpy(&edgeCount, ptr, DATA_COUNT_SIZE);
		ptr += DATA_COUNT_SIZE + edgeCount * CONNECTION_ITEM_SIZE;
	}
}

size_t AdjacencyList::fetchConnectionSetID(const byte* itemStart)
{
	itemStart += COORD_SIZE * 2;

	size_t id = 0;
	memcpy(&id, itemStart, CONNECTION_ID_SIZE);

	return id;
}

void AdjacencyList::fetchCoordinate(const byte* itemStart, MapPos2D* pos)
{
	int x = 0, y = 0;
	memcpy(&x, itemStart, COORD_SIZE);
	itemStart += COORD_SIZE;
	memcpy(&y, itemStart, COORD_SIZE);

	pos->d0 = x;
	pos->d1 = y;
}

void AdjacencyList::fetchConnectionInfo(const byte* itemStart, std::vector<Connection>* conns)
{
	itemStart += COORD_SIZE * 2 + CONNECTION_ID_SIZE;

	size_t cEdge = 0;
	memcpy(&cEdge, itemStart, DATA_COUNT_SIZE);
	itemStart += DATA_COUNT_SIZE;

	conns->reserve(cEdge);
	for (size_t i = 0; i < cEdge; ++i)
	{
		Connection ct;
		memcpy(&ct.nodeID, itemStart, NODE_ID_SIZE);
		itemStart += NODE_ID_SIZE;
		memcpy(&ct.roadID, itemStart, EDGE_ID_SIZE);
		itemStart += EDGE_ID_SIZE;

		size_t len = 0;
		memcpy(&len, itemStart, EDGE_LENGTH_SIZE);
		itemStart += EDGE_LENGTH_SIZE;
		ct.length = decodeTopoEdgeLength(len);

		conns->push_back(ct);
	}
}

size_t AdjacencyList::encodeTopoNodeID(size_t page, size_t offset)
{
	assert(page <= (1 << GRID_SIZE_BIT) - 1
		&& offset <= (1 << OFFSET_SIZE_BIT) - 1);
	return page << OFFSET_SIZE_BIT | offset;
}

void AdjacencyList::decodeTopoNodeID(size_t id, size_t* page, size_t* offset)
{
	*page = (id & TOPO_NODE_GAP) >> OFFSET_SIZE_BIT;
	*offset = (id << GRID_SIZE_BIT) >> GRID_SIZE_BIT;
	// 	offset = id & ( (1 << OFFSET_SIZE_BIT) - 1 );
}

size_t AdjacencyList::encodeTopoEdgeLength(size_t original)
{
	if (original == EDGE_LENGTH_ENDLESS)
		return original;
	else if (original & EDGE_LENGTH_CARRY)
	{
		original >>= EDGE_LENGTH_CONDENSE_BITS;
		if (original & EDGE_LENGTH_CARRY)
		{
			assert(0);
			throw Tools::IllegalArgumentException(
				"AdjacencyList::encodeTopoEdgeLength: over the bound."
				);
		}
		return original | EDGE_LENGTH_CARRY;
	}
	else 
		return original;
}

size_t AdjacencyList::decodeTopoEdgeLength(size_t storage)
{
	if (storage == EDGE_LENGTH_ENDLESS)
		return EDGE_LENGTH_ENDLESS;
	else if (storage & EDGE_LENGTH_CARRY)
		return (storage ^ EDGE_LENGTH_CARRY) << EDGE_LENGTH_CONDENSE_BITS;
	else
		return storage;
}

void AdjacencyList::buildTopoStruct(Publish& pb)
{
	for (size_t i = 0; i < pb.m_layers.size(); ++i)
	{
		STATIC_LAYER* tree = pb.m_layers.at(i);

		std::stack<SimulateNavigate::SpatialIndex::StaticRTree::NodePtr> st;

		SimulateNavigate::SpatialIndex::StaticRTree::NodePtr root 
			= tree->readNode(tree->m_rootID);
		if (root->m_children > 0 && m_leaf->m_scope.intersectsShape(root->m_nodeMBR)) 
			st.push(root);

		while (! st.empty())
		{
			SimulateNavigate::SpatialIndex::StaticRTree::NodePtr n = st.top(); st.pop();

			if (n->isLeaf())
			{
				for (size_t cChild = 0; cChild < n->m_children; ++cChild)
				{
					if (m_leaf->m_scope.intersectsShape(*(n->m_ptrMBR[cChild])) ||
						m_leaf->m_scope.touchesShape(*(n->m_ptrMBR[cChild])))
					{
						IShape* s;
						n->getChildShape(cChild, &s);
						Line* l = dynamic_cast<Line*>(s);
						assert(l != NULL);

						MapPos2D start, end;
						l->getPoint(0, &start);
						l->getPoint(l->posNum() - 1, &end);

						bool bStartIn = PartitionMap::isPointInScope(start, m_leaf->m_scope),
							bEndIn = PartitionMap::isPointInScope(end, m_leaf->m_scope);

						if (bStartIn || bEndIn)
						{
							size_t length = static_cast<size_t>(std::ceil(l->getLength()));
							if (bStartIn)
								insertEdge(start, end, n->m_pIdentifier[cChild], length, bEndIn, pb);
							if (bEndIn)
								insertEdge(end, start, n->m_pIdentifier[cChild], length, bStartIn, pb);
						}

						delete s;
					}
				}
			}
			else
			{
				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (m_leaf->m_scope.intersectsShape(*(n->m_ptrMBR[cChild])) ||
						m_leaf->m_scope.touchesShape(*(n->m_ptrMBR[cChild])))
					{
						st.push(tree->readNode(n->m_pIdentifier[cChild]));
					}
				}
			}
		}
	}
}

void AdjacencyList::partitionScopeToPage(std::vector<std::vector<OnePageData*> >& totPageData, size_t pageSize)
{
	size_t totDataSize = getTotDataSize();
	// Each ALGrid will use 1 byte to store the node's number.
	totDataSize += totDataSize / pageSize;
	size_t gridSize = static_cast<size_t>(std::ceil(static_cast<double>(totDataSize) / static_cast<double>(pageSize)));
	size_t columnSize = static_cast<size_t>(std::ceil(std::pow(static_cast<double>(gridSize), static_cast<double>(0.5))));

	m_leaf->m_firstGridID = m_nextPage;

	size_t totVerGridsSize = columnSize * pageSize;
	TopoNodeLoader::loadTotal(m_nodes, totPageData, m_leaf, m_nextPage, m_buffer, totVerGridsSize, pageSize);
}

void AdjacencyList::outputTopoStruct(const std::vector<std::vector<OnePageData*> >& totPageData, Publish& pb) const
{
	for (size_t i = 0; i < totPageData.size(); ++i)
	{
		for (size_t j = 0; j < totPageData.at(i).size(); ++j)
		{
			byte* page = new byte[pb.m_pageSize];
			totPageData.at(i).at(j)->outPutToPage(page, m_nodes, m_buffer);
			pb.writePageData(m_nextPage++, page);
			delete[] page;
		}
	}
}

void AdjacencyList::insertEdge(const MapPos2D& s, const MapPos2D& e, 
							   uint64_t roadID, size_t roadLength,
							   bool bEndIn, const Publish& pb)
{
	std::set<TopoNode*, TopoNodeHorCmp>::iterator itr 
		= m_nodes.find(&TopoNode(s));

	if (!bEndIn)
	{
		TopoNode* check = new TopoNode(e, false);
		check->m_id = pb.queryTopoNodeID(e);
		m_nodes.insert(check);
	}

	if (itr != m_nodes.end())
	{
		(*itr)->m_relation.push_back(TopoNode::Related(e, roadID, roadLength));
	}
	else
	{
		TopoNode* n = new TopoNode(s);
		n->m_relation.push_back(TopoNode::Related(e, roadID, roadLength));
		m_nodes.insert(n);
	}
}

size_t AdjacencyList::getTotDataSize() const
{
	size_t ret = 0;
	std::set<TopoNode*, TopoNodeHorCmp>::const_iterator itr = m_nodes.begin();
	for ( ; itr != m_nodes.end(); ++itr)
		ret += (*itr)->getDataSize();
	return ret;
}

//
// class AdjacencyList::TopoNode
//

size_t AdjacencyList::TopoNode::getDataSize() const
{
	if (!m_bInScope)
	{
		return 0;
	}
	else
		return 
			COORD_SIZE * 2 + 
			CONNECTION_ID_SIZE +
			DATA_COUNT_SIZE + 
			CONNECTION_ITEM_SIZE * m_relation.size();
}

//
// class AdjacencyList::OnePageData
//

void AdjacencyList::OnePageData::outPutToPage(
	byte* page, 
	const std::set<TopoNode*, TopoNodeHorCmp>& totNodes, 
	TopoNodeBuffer& topoNodeBuffer
	) const
{
	size_t dataCount = m_topoNodes.size();
	assert(dataCount <= MAX_DATA_COUNT);
	memcpy(page, &dataCount, DATA_COUNT_SIZE);
	page += DATA_COUNT_SIZE;

	for (size_t i = 0; i < m_topoNodes.size(); ++i)
	{
		const TopoNode* n = m_topoNodes.at(i);
		assert(n->m_bInScope && n->m_id != 0);

		int x, y;
		x = static_cast<int>(n->m_pos.d0 + 0.5);
		y = static_cast<int>(n->m_pos.d1 + 0.5);

		assert(sizeof(int) == COORD_SIZE);
		memcpy(page, &x, COORD_SIZE);
		page += COORD_SIZE;
		memcpy(page, &y, COORD_SIZE);
		page += COORD_SIZE;

		memset(page, CONNECTION_ID_NULL, CONNECTION_ID_SIZE);
		page += CONNECTION_ID_SIZE;

		size_t edgeCount = n->m_relation.size();
		memcpy(page, &edgeCount, DATA_COUNT_SIZE);
		page += DATA_COUNT_SIZE;

		for (size_t j = 0; j < edgeCount; ++j)
		{
			const TopoNode::Related& r = n->m_relation.at(j);

			size_t length = AdjacencyList::encodeTopoEdgeLength(r.m_roadLength);

			std::set<TopoNode*, TopoNodeHorCmp>::const_iterator itr
				= totNodes.find(&TopoNode(r.m_pos));
			assert(itr != totNodes.end());

			size_t nodeID = (*itr)->m_id;
			if (nodeID == 0)
			{
				assert((*itr)->m_bInScope == false);
				topoNodeBuffer.addUnknownNode(r.m_pos, n->m_id, r.m_roadID, length);
				memcpy(page, &nodeID, NODE_ID_SIZE);
				page += CONNECTION_ITEM_SIZE;
			}
			else
			{
				memcpy(page, &nodeID, NODE_ID_SIZE);
				page += NODE_ID_SIZE;
				memcpy(page, &r.m_roadID, EDGE_ID_SIZE);
				page += EDGE_ID_SIZE;
				memcpy(page, &length, EDGE_LENGTH_SIZE);
				page += EDGE_LENGTH_SIZE;
			}
		}
	}
}

//
// class AdjacencyList::TopoNodeLoader
//

void AdjacencyList::TopoNodeLoader::loadTotal(
	const std::set<TopoNode*, TopoNodeHorCmp>& nodes,
	std::vector<std::vector<OnePageData*> >& data, 
	Leaf* leaf,
	size_t nextPage,
	TopoNodeBuffer& topoNodeBuffer,
	size_t verSize, 
	size_t pageSize
	)
{
	size_t currSize = 0;
	std::set<TopoNode*, TopoNodeVerCmp> verNodes;

	std::set<TopoNode*, TopoNodeHorCmp>::const_iterator itr = nodes.begin();
	for ( ; itr != nodes.end(); ++itr)
	{
		TopoNode* n = *itr;
		if (!n->m_bInScope)
			continue;
		size_t nSize = n->getDataSize();
		if (currSize > verSize)
		{
			std::vector<double> horBound;
			std::vector<OnePageData*> horPages;

			loadVertical(verNodes, pageSize, horBound, horPages, nextPage, topoNodeBuffer);

			double verBound = 0.5 * (n->m_pos.d0 + (*--itr)->m_pos.d0);
			++itr;
			leaf->m_verPartition.push_back(verBound);
			leaf->m_horPartition.push_back(horBound);
			data.push_back(horPages);

			currSize = nSize;
			verNodes.clear();
			verNodes.insert(n);
		}
		else
		{
			currSize += nSize;
			verNodes.insert(n);
		}
	}

	if (!verNodes.empty())
	{
		std::vector<double> horBound;
		std::vector<OnePageData*> horPages;

		loadVertical(verNodes, pageSize, horBound, horPages, nextPage, topoNodeBuffer);

		leaf->m_horPartition.push_back(horBound);
		data.push_back(horPages);
	}
}

void AdjacencyList::TopoNodeLoader::loadVertical(
	const std::set<TopoNode*, TopoNodeVerCmp>& nodes,
	size_t pageSize,
	std::vector<double>& horBound,
	std::vector<OnePageData*>& horPages,
	size_t& nextPage,
	TopoNodeBuffer& topoNodeBuffer
	)
{
	size_t currSize = DATA_COUNT_SIZE;
	OnePageData* pageData = new OnePageData;

	std::set<TopoNode*, TopoNodeVerCmp>::const_iterator itr = nodes.begin();
	for ( ; itr != nodes.end(); ++itr)
	{
		TopoNode* n = *itr;
		if (!n->m_bInScope)
			continue;
		size_t nSize = n->getDataSize();
		if (currSize + nSize > pageSize)
		{
			double hor = 0.5 * (n->m_pos.d1 + pageData->m_topoNodes.back()->m_pos.d1);
			horBound.push_back(hor);
			horPages.push_back(pageData);

			currSize = nSize + DATA_COUNT_SIZE;
			pageData = new OnePageData;

			n->m_id = encodeTopoNodeID(++nextPage, DATA_COUNT_SIZE);

			pageData->m_topoNodes.push_back(n);
			topoNodeBuffer.eraseUnknowNode(n->m_pos, n->m_id);
		}
		else
		{
			n->m_id = encodeTopoNodeID(nextPage, currSize);

			currSize += nSize;

			pageData->m_topoNodes.push_back(n);
			topoNodeBuffer.eraseUnknowNode(n->m_pos, n->m_id);
		}
	}

	if (!pageData->m_topoNodes.empty())
	{
		horPages.push_back(pageData);
		++nextPage;
	}
}