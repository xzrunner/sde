#include "MapPartition.h"
#include "Index.h"
#include "Leaf.h"
#include "Publish.h"
#include "../../SpatialIndex/NVRTree/NVRTree.h"
#include "../../SpatialIndex/Utility/QueryStrategy.h"
#include "../../BasicType/Line.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

PartitionMap::PartitionMap(const std::vector<NVDataPublish::STATIC_LAYER*>& src, size_t maxNum)
	: m_layers(src), m_maxNum(maxNum), m_root(NULL)
{
	build();
}

PartitionMap::~PartitionMap()
{
	delete m_root;
}

void PartitionMap::queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const
{
	m_root->queryAdjacencyListID(p, IDs);
}

void PartitionMap::storeToStorageManager(Publish& pb, size_t pageSize, size_t& nextPage) const
{
	typedef size_t NODE_ID;
	typedef size_t NODE_COUNT;
	typedef std::pair<NODE_ID, NODE_COUNT>	PAIR_ID_COUNT;

	if (m_root->isLeaf())
		return;

	size_t groupsPerPage = static_cast<size_t>(std::floor(
		static_cast<double>(pageSize) 
		/ static_cast<double>(4 * (Node::PM_NODE_ID_SIZE + Node::PM_NODE_COUNT_SIZE))
		));

	std::stack<Index*> buffer;

	std::vector<std::vector<PAIR_ID_COUNT> > totData;
	std::vector<PAIR_ID_COUNT> pageData;
	pageData.reserve(groupsPerPage * 4);

	std::map<Index*, PAIR_ID_COUNT> notSetID;

	buffer.push(dynamic_cast<Index*>(m_root));
	while (!buffer.empty())
	{
		Index* n = buffer.top(); buffer.pop();

		std::map<Index*, PAIR_ID_COUNT>::iterator itr
			= notSetID.find(n);
		if (itr != notSetID.end())
		{
			if (itr->second.first < totData.size())
				assert(totData.at(itr->second.first).at(itr->second.second).first == 0);
			else
				assert(itr->second.first == totData.size() && pageData.at(itr->second.second).first == 0);

			size_t page = totData.size() + nextPage;
			size_t offset = pageData.size() * (Node::PM_NODE_ID_SIZE + Node::PM_NODE_COUNT_SIZE);
			assert(page <= (1 << Node::GRID_SIZE_BIT) - 1
				&& offset <= (1 << Node::OFFSET_SIZE_BIT) - 1);
			size_t id = (page << Node::OFFSET_SIZE_BIT) | offset;

			if (itr->second.first < totData.size())
				totData.at(itr->second.first).at(itr->second.second) = PAIR_ID_COUNT(id, itr->first->m_nodeCount);
			else
				pageData.at(itr->second.second) = PAIR_ID_COUNT(id, itr->first->m_nodeCount);

			notSetID.erase(itr);
		}

		for (size_t i = 0; i < 4; ++i)
		{
			Node* c = n->m_child[i];
			if (c->isLeaf())
			{
				size_t leafID = dynamic_cast<Leaf*>(c)->m_id;
				pageData.push_back(std::make_pair(leafID, c->m_nodeCount));
			}
			else
			{
				Index* tmp = dynamic_cast<Index*>(c);
				buffer.push(tmp);
				assert(notSetID.find(tmp) == notSetID.end());
				notSetID.insert(std::make_pair(tmp, std::make_pair(totData.size(), pageData.size())));
				pageData.push_back(std::make_pair(0, tmp->m_nodeCount));
			}
		}

		if (pageData.size() + 4 > groupsPerPage * 4)
		{
			totData.push_back(pageData);
			pageData.clear();
			pageData.reserve(groupsPerPage * 4);
		}
	}

	if (!pageData.empty())
		totData.push_back(pageData);

	assert(notSetID.empty());

	byte* data = new byte[pageSize];
	for (size_t i = 0; i < totData.size(); ++i)
	{
		byte* ptr = data;
		for (size_t j = 0; j < totData.at(i).size(); ++j)
		{
			memcpy(ptr, &totData.at(i).at(j).first, sizeof(size_t));
			ptr += sizeof(size_t);
			memcpy(ptr, &totData.at(i).at(j).second, sizeof(size_t));
			ptr += sizeof(size_t);
		}

		pb.writePageData(nextPage++, data);
	}
	delete[] data;
}

void PartitionMap::loadFromStorageManager(IStorageManager* sm, size_t firstIndexPage)
{
	// todo
}

bool PartitionMap::isPointInScope(const MapPos2D& p, const Rect& s)
{
	if (p.d0 < s.m_pLow[0] || p.d0 >= s.m_pHigh[0] ||
		p.d1 < s.m_pLow[1] || p.d1 >= s.m_pHigh[1])
		return false;
	else
		return true;
}

void PartitionMap::build()
{
	Rect totScope;
	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		SpatialIndex::LayerRegionQueryStrategy qs;
		m_layers.at(i)->queryStrategy(qs);
		totScope.combineRect(qs.m_indexedSpace);
	}
	totScope.changeSize(1.0, 1.0);

	if (shouldSplitNode(totScope))
	{
		m_root = new Index(totScope);
		m_root->split(*this);
	}
	else
	{
		m_root = new Leaf(totScope);
		m_allLeaves.push_back(m_root);
	}
}

bool PartitionMap::shouldSplitNode(const Rect& scope) const
{
	CountNodeVisitor vis(m_maxNum);

	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		if (m_layers.at(i)->countIntersectsQuery(scope, vis))
			return true;
	}
	return false;
}

//
// class PartitionMap::CountRoadVisitor
//

void PartitionMap::CountNodeVisitor::countNode(const INode& node)
{
	m_count += node.getChildrenCount() * 2;	
}

void PartitionMap::CountNodeVisitor::countData(const INode& node, size_t iData, const IShape& query)
{
	IShape* s;
	node.getChildShape(iData, &s);
	Line* l = dynamic_cast<Line*>(s);
	assert(l);

	const Rect* r = dynamic_cast<const Rect*>(&query);
	assert(r);

	MapPos2D p;
	l->getPoint(0, &p);
	if (PartitionMap::isPointInScope(p, *r))
		++m_count;
	l->getPoint(l->posNum() - 1, &p);
	if (PartitionMap::isPointInScope(p, *r))
		++m_count;

	delete s;
}

bool PartitionMap::CountNodeVisitor::overUpperLimit() const
{
	return m_count > m_upperLimit;
}