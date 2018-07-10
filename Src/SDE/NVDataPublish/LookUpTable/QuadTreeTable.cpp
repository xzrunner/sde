#include "QuadTreeTable.h"
#include "TableBuilding.h"
#include "Publish.h"
#include "../Network/AdjacencyList.h"
#include "../Utility/QuadTreePartition.h"
#include "../../SimulateNavigate/Network/SLSpatialIndex.h"
#include "../../Tools/SpatialMath.h"
#include "../../Render/Render.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::LookUpTable;

typedef NVDataPublish::Utility::QPNode QPNode;

QuadTreeTable::QuadTreeTable(IStorageManager& storage)
	: m_buffer(&storage)
{
	loadHeader();
}

QuadTreeTable::QuadTreeTable(IStorageManager& tableStorage, IStorageManager& nwStorage, 
							 size_t capacity, const MapScope& scope)
	: m_buffer(&tableStorage)
{
	initTableIndex(nwStorage, capacity, scope);
}

QuadTreeTable::~QuadTreeTable()
{
	delete m_root;
}

double QuadTreeTable::queryMinDistance(const MapPos2D& sNodePos, size_t dGridID) const
{
	return TableBuilding::queryMinDistance(m_buffer, m_dataStartPage, m_gridNum, 
		getGridID(sNodePos), dGridID);
}

void QuadTreeTable::loadTable(size_t destGridID, byte** data) const
{
	return TableBuilding::loadTable(m_buffer, m_dataStartPage, m_gridNum, destGridID, data);
}

double QuadTreeTable::queryMinDistance(const MapPos2D& src, const byte* table) const
{
	return TableBuilding::queryMinDistance(getGridID(src), table);
}

size_t QuadTreeTable::getGridID(const MapPos2D& pos) const
{
	Node* curr = m_root;
	MapScope scope(m_mapScope);
	while (true)
	{
		assert(Tools::SpatialMath::isPointInRect(pos, scope));
		if (curr->isLeaf())
			return curr->m_id;
		else
		{
			double cx = 0.5 * (scope.m_xMin + scope.m_xMax),
				cy = 0.5 * (scope.m_yMin + scope.m_yMax);
			if (pos.d0 < cx)
			{
				if (pos.d1 < cy)
				{
					curr = curr->m_child[2];
					scope.m_xMax = cx; scope.m_yMax = cy;
				}
				else
				{
					curr = curr->m_child[0];
					scope.m_xMax = cx; scope.m_yMin = cy;
				}
			}
			else
			{
				if (pos.d1 < cy)
				{
					curr = curr->m_child[3];
					scope.m_xMin = cx; scope.m_yMax = cy;
				}
				else
				{
					curr = curr->m_child[1];
					scope.m_xMin = cx; scope.m_yMin = cy;
				}
			}
		}
	}

	assert(0);
	throw Tools::IllegalStateException(
		"QuadTreeTable::getGridID: Didn't find."
		);
	return Tools::UINT32_MAX;
}

void QuadTreeTable::outputIndexInfo(size_t& nextPage)
{
	storeHeader();
	nextPage = m_dataStartPage;
}

void QuadTreeTable::drawTable(Render::GDIRender* render) const
{
	HDC tmpDC = render->getMemDC();
	SelectObject(tmpDC, CreatePen(PS_DASH, 1, RGB(255, 0, 0)));

	std::queue<std::pair<Node*, Rect> > buffer;
	buffer.push(std::make_pair(m_root, Rect(m_mapScope.leftLow(), m_mapScope.rightTop())));

	while (!buffer.empty())
	{
		std::pair<Node*, Rect> item = buffer.front(); buffer.pop();

		if (item.first->isLeaf())
			render->render(item.second);
		else
		{
			Rect childScope(item.second);
			for (size_t i = 0; i < 4; ++i)
			{
				Tools::SpatialMath::getRectChildGrid(item.second, i, childScope);
				buffer.push(std::make_pair(item.first->m_child[i], childScope));
			}
		}
	}

	DeleteObject(SelectObject(tmpDC, GetStockObject(BLACK_PEN)));
	render->endMemRender();
}

//
// class QuadTreeTable::Node
//

QuadTreeTable::Node::Node(const QPNode* src)
	: m_id(Tools::UINT32_MAX)
{
	if (src->isLeaf())
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = NULL;
	else
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = new Node(src->getChild(i));
}

QuadTreeTable::Node::Node(byte** ptr)
{
	size_t id = 0;
	memcpy(&id, *ptr, QuadTreeTable::NODE_ID_SIZE);
	*ptr += QuadTreeTable::NODE_ID_SIZE;

	if (id == QuadTreeTable::NODE_ID_ENDLESS)
	{
		m_id = Tools::UINT32_MAX;
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = new Node(ptr);
	}
	else
	{
		m_id = id;
		for (size_t i = 0; i < 4; ++i)
			m_child[i] = NULL;
	}
}

QuadTreeTable::Node::~Node()
{
	for (size_t i = 0; i < 4; ++i)
		delete m_child[i];
}

void QuadTreeTable::Node::countAndSetIDInPreorder(size_t& c)
{
	if (isLeaf())
		m_id = c++;
	else
	{
		for (size_t i = 0; i < 4; ++i)
			m_child[i]->countAndSetIDInPreorder(c);
	}
}

void QuadTreeTable::Node::preorderTraversal(std::vector<const Node*>& list) const
{
	list.push_back(this);
	if (!isLeaf())
	{
		for (size_t i = 0; i < 4; ++i)
			m_child[i]->preorderTraversal(list);
	}
}

//
// Internal
//

void QuadTreeTable::storeHeader()
{
	const size_t headerSize =
		sizeof(size_t)		+		// m_gridNum
		sizeof(double) * 4	+		// m_mapScope
		sizeof(size_t);				// m_dataStartPage

	byte* header = new byte[headerSize];
	byte* ptr = header;

	memcpy(ptr, &m_gridNum, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_mapScope.m_xMin, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_yMin, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_xMax, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_mapScope.m_yMax, sizeof(double));
	ptr += sizeof(double);

	std::vector<const Node*> nodes;
	m_root->preorderTraversal(nodes);
	const size_t indexSize = nodes.size() * NODE_ID_SIZE;
	m_dataStartPage = static_cast<size_t>( std::ceil( 
		static_cast<double>(indexSize) / static_cast<double>(Publish::PAGE_SIZE) 
		) ) + 1;
	memcpy(ptr, &m_dataStartPage, sizeof(size_t));
	ptr += sizeof(size_t);

	id_type id = HEADER_PAGE;
	m_buffer->storeByteArray(id, headerSize, header);
	delete[] header;

	byte* indexData = new byte[indexSize];
	ptr = indexData;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const Node* n = nodes[i];
		if (n->isLeaf())
		{
			assert(n->m_id < NODE_ID_GAP);
			memcpy(ptr, &n->m_id, NODE_ID_SIZE);
		}
		else
			memcpy(ptr, &NODE_ID_ENDLESS, NODE_ID_SIZE);
		ptr += NODE_ID_SIZE;
	}

	byte* pageData = new byte[Publish::PAGE_SIZE];
	ptr = indexData;
	size_t cLen;
	size_t cRem = indexSize;
	id_type currPage = HEADER_PAGE + 1;

	do 
	{
		cLen = (cRem > Publish::PAGE_SIZE) ? Publish::PAGE_SIZE : cRem;
		memcpy(pageData, ptr, cLen);
		m_buffer->storeByteArray(currPage, cLen, pageData);
		ptr += cLen;
		cRem -= cLen;
		++currPage;

	} while (cRem != 0);

	delete[] pageData;
	delete[] indexData;
}

void QuadTreeTable::loadHeader()
{
	size_t headerSize;
	byte* header = 0;
	m_buffer->loadByteArray(HEADER_PAGE, headerSize, &header);

	byte* ptr = header;

	memcpy(&m_gridNum, ptr, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(&m_mapScope.m_xMin, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_yMin, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_xMax, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_mapScope.m_yMax, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_dataStartPage, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	delete[] header;

	size_t indexSize = ( m_dataStartPage - 1 ) * Publish::PAGE_SIZE;
	byte* indexData = new byte[indexSize];
	ptr = indexData;
	for (size_t i = 1; i < m_dataStartPage; ++i)
	{
		size_t pageSize;
		byte* pageData = 0;
		m_buffer->loadByteArray(i, pageSize, &pageData);

		byte* ptrSrc = pageData;
		memcpy(ptr, ptrSrc, Publish::PAGE_SIZE);
		ptr += Publish::PAGE_SIZE;

		delete[] pageData;
	}

	ptr = indexData;
	m_root = new Node(&ptr);

	delete[] indexData;
}

void QuadTreeTable::initTableIndex(IStorageManager& nwStorage, size_t capacity, const MapScope& scope)
{
	SimulateNavigate::Network::SLSpatialIndex srcQuadTree(nwStorage);
	m_mapScope = srcQuadTree.m_scope;

	QPNode* tmpRoot = new QPNode(srcQuadTree.m_root);
	tmpRoot->reconstruct(&nwStorage, capacity);

	m_root = new Node(tmpRoot);
	delete tmpRoot;

	m_gridNum = 0;
	m_root->countAndSetIDInPreorder(m_gridNum);
}
