#include "Publish.h"
#include "MapPartition.h"
#include "MidGraph.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "EnterGridNodesMapping.h"
#include "../Network/Publish.h"
#include "../../SimulateNavigate/Network/SingleLayerNW.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW; 

const std::wstring Publish::FILE_NAME= L"MultiLayersNW";

Publish::Publish(Tools::PropertySet& ps)
{
	Tools::Variant var;

	var = ps.getProperty("FilePath");
	assert (var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_PWCHAR)
		throw Tools::IllegalArgumentException(
		"StorageManager: Property FilePath must be Tools::VT_PWCHAR"
		);
	std::wstring singlePath = std::wstring(var.m_val.pwcVal) + L"\\" + NVDataPublish::Network::Publish::FILE_NAME,
		multiPath = std::wstring(var.m_val.pwcVal) + L"\\" + FILE_NAME;

	var = ps.getProperty("PageSize");
	assert(var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_ULONG)
		throw Tools::IllegalArgumentException(
		"Property PageSize must be Tools::VT_ULONG"
		);
	m_pageSize = var.m_val.ulVal;

	m_singleFile = NVDataPublish::Network::Publish::loadNWStorageManager(singlePath);
	m_multiFile = StorageManager::createNewStaticDiskStorageManager(multiPath, m_pageSize);

	m_singleBuffer = StorageManager::createNewFIFOBuffer(*m_singleFile, 1000, false);
	m_multiBuffer = StorageManager::createNewFIFOBuffer(*m_multiFile, 1000, false);

	var = ps.getProperty("MaxNodeNumToCondense");
	if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 2)
		throw Tools::IllegalArgumentException(
		"LookUpTable::Publish: Property MaxNodeNumPerLookUpTable must be Tools::VT_ULONG and >= 2"
		);
	size_t capacity = var.m_val.lVal;

	m_partition = new PartitionMap(*m_singleBuffer, capacity);
}

Publish::~Publish()
{
	delete m_partition;
	delete m_multiBuffer;
	delete m_singleBuffer;
	delete m_multiFile;
	delete m_singleFile;
}

void Publish::createMultiLayersNWData(HWND hwnd)
{
	id_type nextPage = HEADER_PAGE;
	// <1> Header
	size_t indexPageCount = 0;
	size_t tablePageStart = 0;
	storeHeader(indexPageCount, tablePageStart, nextPage);
	// <2> QuadTree index
	indexPageCount = static_cast<size_t>(nextPage);
	storeQuadtreeIndex(nextPage);
	indexPageCount = static_cast<size_t>(nextPage) - indexPageCount;
	// <3> Each grid's data
	createAndStoreMultiAL(hwnd, nextPage);
	// <4> HighID map
	tablePageStart = static_cast<size_t>(nextPage);
	storeComeInNodesLookUpTable(nextPage);
	id_type headerPage = HEADER_PAGE;
	storeHeader(indexPageCount, tablePageStart, headerPage);
}

void Publish::storeHeader(size_t indexPageCount, size_t tablePageStart, id_type& nextPage)
{
	byte* data = new byte[m_pageSize];
	byte* ptr = data;

	memcpy(ptr, &m_pageSize, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &indexPageCount, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &tablePageStart, sizeof(size_t));
	ptr += sizeof(size_t);

	m_multiBuffer->storeByteArray(nextPage, m_pageSize, data);
	++nextPage;

	delete[] data;
}

void Publish::storeQuadtreeIndex(id_type& nextPage)
{
	byte* totData;
	size_t totLen;
	m_partition->storeToByteArray(&totData, totLen);

	byte* data = new byte[m_pageSize];
	size_t rem = totLen, len;
	byte* ptr = totData;
	do 
	{
		len = rem <= m_pageSize ? rem : m_pageSize;

		memcpy(data, ptr, len);
		ptr += len;

		rem -= len;
		m_multiBuffer->storeByteArray(nextPage, m_pageSize, data);
		++nextPage;

	} while (rem != 0);

	delete[] data;
	delete[] totData;
}

void Publish::createAndStoreMultiAL(HWND hwnd, id_type& nextPage)
{
	m_partition->setAllGridsEnterNodeInfo(*m_singleBuffer);

	std::vector<Node*> grids;
	m_partition->getAllGridsByPostorderTraversal(grids);

	//// debug
	//// for test
	//std::ofstream fout("c:/3_21.txt");
	//fout << grids.size() << std::endl;
	////

	size_t cGrid = grids.size();
	for (size_t i = 0; i < cGrid; ++i)
	{
		// display progress
		char positionBuffer[100];
		sprintf(positionBuffer, "%d / %d", i, cGrid);
		HDC hdc = GetDC(hwnd);
		TextOutA(hdc, 10, 0, positionBuffer, strlen(positionBuffer));
		ReleaseDC(hwnd, hdc);

		Node* n = grids[i];
		MidGraph* g = new MidGraph(n);

		if (n->isLeaf())
		{
			const Leaf* leaf = dynamic_cast<const Leaf*>(n);
			g->initFromSingleNWStorage(*m_singleBuffer, leaf->m_singleALPageIDs);
		}
		else
		{
			std::vector<size_t> gridIDs;
			const Index* parent = dynamic_cast<const Index*>(n);
			for (size_t i = 0; i < 4; ++i)
			{
				const Node* child = parent->m_child[i];
				for (size_t j = 0; j < child->m_pageCount; ++j)
					gridIDs.push_back(static_cast<size_t>(child->m_startPage) + j);
			}

			g->initFromMultiNWStorage(*m_singleBuffer, *m_multiBuffer, gridIDs);
		}

		g->condenseBySearchingShortestPath();

		n->m_startPage = nextPage;
		g->outputCondensedGraph(*m_multiBuffer, m_pageSize, nextPage);
		n->m_pageCount = static_cast<size_t>(nextPage - n->m_startPage);

		//// for test
		//fout << n->m_startPage << " " << n->m_pageCount << std::endl;
		////

		delete g;
	}
}

void Publish::storeComeInNodesLookUpTable(id_type& nextPage)
{
	EnterGridNodesMapping mapping(m_partition->getRoot());
	mapping.outputToStorage(*m_multiBuffer, m_pageSize, nextPage);
}