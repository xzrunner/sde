#include "MapPartition.h"
#include "Leaf.h"
#include "Index.h"
#include "AdjacencyList.h"
#include "../Utility/QuadTreePartition.h"
#include "../Network/AdjacencyList.h"
#include "../Network/Publish.h"
#include "../../SimulateNavigate/Network/SLSpatialIndex.h"
#include "../../SimulateNavigate/Network/SingleLayerNW.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW; 

typedef NVDataPublish::Utility::QPNode			QPNode;
typedef NVDataPublish::Network::AdjacencyList	S_AL;
typedef AdjacencyList							M_AL;


PartitionMap::PartitionMap(IStorageManager& sm, size_t capacity)
	: m_singleNW(new SimulateNavigate::Network::SingleLayerNW(&sm))
{
	SimulateNavigate::Network::SLSpatialIndex srcQuadTree(sm);
	m_scope = srcQuadTree.m_scope;
	m_scope.changeSize(1.0);

	QPNode* tmpRoot = new QPNode(srcQuadTree.m_root);
	tmpRoot->reconstruct(&sm, capacity);

	if (tmpRoot->isLeaf())
		m_root = new Leaf(m_scope, tmpRoot->getSingleALPageIDs());
	else
		m_root = new Index(tmpRoot, m_scope);
	delete tmpRoot;
}

PartitionMap::~PartitionMap()
{
	delete m_root;
	delete m_singleNW;
}

void PartitionMap::setAllGridsEnterNodeInfo(IStorageManager& sm)
{
	size_t lPage, iPage;
	dynamic_cast<SimulateNavigate::Network::SingleLayerNW*>(m_singleNW)->getStorageInfo(&lPage, &iPage);
	size_t page = NVDataPublish::Network::Publish::BEGIN_AL_PAGE;
	for ( ; page < lPage; ++page)
	{
		byte* data;
		size_t len;
		sm.loadByteArray(page, len, &data);

		byte* ptr = data;

		size_t cData = 0;
		memcpy(&cData, ptr, S_AL::DATA_COUNT_SIZE);
		ptr += S_AL::DATA_COUNT_SIZE;
		for (size_t iData = 0; iData < cData; ++iData)
		{
			MapPos2D fPos;
			S_AL::fetchCoordinate(ptr, &fPos);
			ptr += S_AL::COORD_SIZE * 2 + S_AL::CONNECTION_ID_SIZE;

			size_t cEdge = 0;
			memcpy(&cEdge, ptr, S_AL::DATA_COUNT_SIZE);
			ptr += S_AL::DATA_COUNT_SIZE;
			for (size_t iEdge = 0; iEdge < cEdge; ++iEdge)
			{
				size_t tNodeID = 0;
				memcpy(&tNodeID, ptr, S_AL::NODE_ID_SIZE);
				MapPos2D tPos;
				M_AL::getTopoNodePos(sm, tNodeID, &tPos);
				
				insertEnterGridItem(fPos, tPos, tNodeID);

				ptr += S_AL::CONNECTION_ITEM_SIZE;
			}
		}

		delete[] data;
	}
}

void PartitionMap::getAllGridsByPostorderTraversal(std::vector<Node*>& grids)
{
	grids.clear();
	m_root->postorderTraversal(grids);
}

void PartitionMap::storeToByteArray(byte** data, size_t& length) const
{
	std::vector<bool> tmpData;
	std::stack<Node*> buffer;
	buffer.push(m_root);
	while (!buffer.empty())
	{
		Node* n = buffer.top(); buffer.pop();
		if (n->isLeaf())
			tmpData.push_back(false);
		else
		{
			tmpData.push_back(true);
			Index* index = dynamic_cast<Index*>(n);
			for (int i = 3; i >= 0; --i)
				buffer.push(index->m_child[i]);
		}
	}

	length = 
		sizeof(double) * 4 + 
		static_cast<size_t>(std::ceil(static_cast<double>(tmpData.size()) / 8.0));
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, m_root->m_scope.m_pLow, sizeof(double) * 2);
	ptr += sizeof(double) * 2;
	memcpy(ptr, m_root->m_scope.m_pHigh, sizeof(double) * 2);
	ptr += sizeof(double) * 2;

	byte group = 0;
	size_t rem = 8;
	for (size_t i = 0; i < tmpData.size(); ++i)
	{
		group = ( group << 1 ) | ( tmpData[i] ? 0x01 : 0x00 );
		--rem;
		if (rem == 0)
		{
			memcpy(ptr, &group, sizeof(byte));
			ptr += sizeof(byte);
			group = 0;
			rem = 8;
		}
	}
	if (rem != 0)
		memcpy(ptr, &group, sizeof(byte));
}

void PartitionMap::insertEnterGridItem(const MapPos2D& s, const MapPos2D& e, size_t eID)
{
	Node* split = m_root;
	do 
	{
		if (split->isLeaf())
			break;

		size_t sPos = Tools::SpatialMath::getPositionPosInRect(split->m_scope, s),
			ePos = Tools::SpatialMath::getPositionPosInRect(split->m_scope, e);
		Index* index = dynamic_cast<Index*>(split);
		assert(index);
		if (sPos == ePos)
			split = index->m_child[sPos];
		else
		{
			Node* curr = index->m_child[ePos];
			do 
			{
				curr->m_transEnterGridNodeIDs.insert(std::make_pair(eID, 0));
				if (curr->isLeaf())
					break;
				else
				{
					size_t pos = Tools::SpatialMath::getPositionPosInRect(curr->m_scope, e);
					Index* index = dynamic_cast<Index*>(curr);
					curr = index->m_child[pos];
				}
			} while (true);
			break;
		}
	} while (true);
}
