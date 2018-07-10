#include "Publish.h"
#include "MapPartition.h"
#include "AdjacencyList.h"
#include "TopoNodeBuffer.h"
#include "Leaf.h"
#include "ConnectionSet.h"
#include "../Features/Publish.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"
#include "../../Tools/StringTools.h"
#include "../../Render/MapLayer.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Network;

const std::wstring Publish::FILE_NAME= L"AdjacentList";

Publish::Publish(const NVDataPublish::Features::Publish& fp, Tools::PropertySet& ps)
{
	fp.getReconstructedLayers(m_layers);
	fp.getLayersName(m_layersName);
	init(ps);
}

Publish::Publish(const std::vector<Render::MapLayer*>& layers, Tools::PropertySet& ps)
{
	m_layersName.reserve(layers.size());
	for (size_t i = 0; i < layers.size(); ++i)
	{
		STATIC_LAYER* layer = dynamic_cast<STATIC_LAYER*>(layers[i]->getSpatialIndex());
		assert(layer);
		m_layers.push_back(layer);
		m_layersName.push_back(layers[i]->getLayerName());
	}
	
	init(ps);
}

Publish::~Publish()
{
	delete m_partitionMap;
	delete m_buffer;
	delete m_file;
}

void Publish::createNetworkData()
{
	size_t nextPage = 0;

	byte* data = new byte[m_pageSize];
	writePageData(nextPage++, data);
	delete[] data;

	createTopoData(nextPage);

	size_t firstLeafPage = nextPage;
	createLeafData(nextPage);

	m_partitionMap->m_root->setNodeCount();

	size_t firstIndexPage = nextPage;
	createIndexData(nextPage);
	bool rootIsLeaf = false;
	if (nextPage == firstIndexPage)
		rootIsLeaf = true;

	createHeader(firstLeafPage, firstIndexPage, rootIsLeaf);
}

size_t Publish::queryTopoNodeID(const MapPos2D& p) const
{
	std::vector<size_t> IDs;
	m_partitionMap->queryAdjacencyListID(p, &IDs);
	if (IDs.empty())
		return 0;

	for (size_t i = 0; i < IDs.size(); ++i)
	{
		byte* data;
		readPageData(IDs.at(i), &data);

		size_t offset = AdjacencyList::computeDataOffset(p, data);
		delete[] data;
		if (offset != 0)
			return AdjacencyList::encodeTopoNodeID(IDs.at(i), offset);
	}

	assert(0);
	throw Tools::IllegalStateException("Publish::queryTopoNodeID: Can't find pos.");
}

void Publish::readPageData(size_t pageID, byte** data) const
{
	size_t len;
	m_buffer->loadByteArray(static_cast<id_type>(pageID), len, data);
}

void Publish::writePageData(size_t pageID, const byte* const data)
{
	id_type id = static_cast<id_type>(pageID);
	m_buffer->storeByteArray(id, m_pageSize, data);
}

void Publish::fetchLayersName(IStorageManager* sm, std::vector<std::wstring>& names)
{
	size_t len;
	byte* data;
	sm->loadByteArray(0, len, &data);

	byte* ptr = data;
	ptr += sizeof(double) * 2 * 2 + sizeof(size_t) * 4;

	size_t layerNum;
	memcpy(&layerNum, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	names.clear();
	names.reserve(layerNum);
	for (size_t i = 0; i < layerNum; ++i)
	{
		names.push_back(Tools::StringTools::readString(ptr, len));
		ptr += len;
	}

	delete[] data;
}

void Publish::init(Tools::PropertySet& ps)
{
	Tools::Variant var;

	var = ps.getProperty("PageSize");
	assert(var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_ULONG)
		throw Tools::IllegalArgumentException(
		"StorageManager: Property PageSize must be Tools::VT_ULONG"
		);
	m_pageSize = var.m_val.ulVal;

	var = ps.getProperty("FilePath");
	assert (var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_PWCHAR)
		throw Tools::IllegalArgumentException(
		"StorageManager: Property FilePath must be Tools::VT_PWCHAR"
		);
	std::wstring path = std::wstring(var.m_val.pwcVal) + L"\\" + FILE_NAME;
	m_file = StorageManager::createNewStaticDiskStorageManager(path, m_pageSize);

	var = ps.getProperty("Capacity");
	assert (var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_ULONG) 
		throw Tools::IllegalArgumentException("Property Capacity must be Tools::VT_ULONG");
	size_t capacity = var.m_val.ulVal;

	var = ps.getProperty("WriteThrough");
	assert (var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_BOOL) 
		throw Tools::IllegalArgumentException("Property WriteThrough must be Tools::VT_BOOL");
	bool bWriteThrough = var.m_val.blVal;
	m_buffer = StorageManager::createNewFIFOBuffer(*m_file, capacity, bWriteThrough);

	var = ps.getProperty("MaxComputeNodeNum");
	if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 2)
		throw Tools::IllegalArgumentException(
		"NVDataPublish::publishData: Property MaxComputeNum must be Tools::VT_ULONG and >= 2"
		);
	size_t maxComputeNum = var.m_val.lVal;
	m_partitionMap = new PartitionMap(m_layers, maxComputeNum);
}

void Publish::createTopoData(size_t& nextPage)
{
	TopoNodeBuffer topoBuffer(*this);
	for (size_t i = 0; i < m_partitionMap->m_allLeaves.size(); ++i)
	{
		Leaf* leaf = dynamic_cast<Leaf*>(m_partitionMap->m_allLeaves.at(i));

		AdjacencyList* al = new AdjacencyList(leaf, topoBuffer, nextPage);
		al->createTopoData(*this, leaf->m_nodeCount);
		delete al;

		leaf->setBuilt();
	}
	topoBuffer.complementTopoNodeInfo();

	ConnectionSet::setStrongConnectionSetID(this, nextPage);
}

void Publish::createLeafData(size_t& nextPage)
{
	byte* data = new byte[m_pageSize];
	byte* curr = data + LEAF_COUNT_SIZE;
	size_t count = 0;

	for (size_t i = 0; i < m_partitionMap->m_allLeaves.size(); ++i)
	{
		Leaf* l = dynamic_cast<Leaf*>(m_partitionMap->m_allLeaves.at(i));
		assert(l);

		if (l->hasNoData())
		{
			l->setID(0, 0);
			continue;
		}

		byte* leafData;
		size_t leafSize;
		l->storeToByteArray(&leafData, leafSize);
		if (leafSize >= m_pageSize)
		{
			delete[] leafData;
			assert(0);
			throw Tools::IllegalStateException("Publish::createLeafData: Leaf size larger than PageSize.");
		}

		size_t offset = static_cast<size_t>(curr - data);
		if (offset + leafSize > m_pageSize)
		{
			assert(count < AdjacencyList::MAX_DATA_COUNT);
			memcpy(data, &count, LEAF_COUNT_SIZE);
			writePageData(nextPage++, data);

			curr = data + LEAF_COUNT_SIZE;
			count = 0;

			offset = LEAF_COUNT_SIZE;
		}

		l->setID(nextPage, offset);
		++count;
		memcpy(curr, leafData, leafSize);
		curr += leafSize;
		delete[] leafData;
	}

	if (count != 0)
	{
		assert(count < AdjacencyList::MAX_DATA_COUNT);
		memcpy(data, &count, LEAF_COUNT_SIZE);
		writePageData(nextPage++, data);
	}

	delete[] data;
}

void Publish::createIndexData(size_t& nextPage)
{
	m_partitionMap->storeToStorageManager(*this, m_pageSize, nextPage);
}

void Publish::createHeader(size_t firstLeafPage, size_t firstIndexPage, bool rootIsLeaf)
{
	byte* data = new byte[m_pageSize];
	byte* ptr = data;

	memcpy(ptr, m_partitionMap->m_root->m_scope.m_pLow, sizeof(double) * 2);
	ptr += sizeof(double) * 2;
	memcpy(ptr, m_partitionMap->m_root->m_scope.m_pHigh, sizeof(double) * 2);
	ptr += sizeof(double) * 2;

	size_t bLeaf = rootIsLeaf ? 1 : 0;
	memcpy(ptr, &bLeaf, sizeof(size_t));
	ptr += sizeof(size_t);

	assert(firstLeafPage < Tools::UINT32_MAX);
	memcpy(ptr, &firstLeafPage, sizeof(size_t));
	ptr += sizeof(size_t);

	assert(firstIndexPage < Tools::UINT32_MAX);
	memcpy(ptr, &firstIndexPage, sizeof(size_t));
	ptr += sizeof(size_t);

	memcpy(ptr, &m_partitionMap->m_root->m_nodeCount, sizeof(size_t));
	ptr += sizeof(size_t);

	size_t layerNum = m_layersName.size();
	memcpy(ptr, &layerNum, sizeof(size_t));
	ptr += sizeof(size_t);
	for (size_t i = 0; i < layerNum; ++i)
		Tools::StringTools::writeString(m_layersName[i], &ptr);

	writePageData(HEADER_PAGE, data);

	delete[] data;
}