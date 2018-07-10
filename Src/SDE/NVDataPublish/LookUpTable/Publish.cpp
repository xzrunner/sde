#include "Publish.h"
#include "TableBuilding.h"
#include "RegularGridTable.h"
#include "QuadTreeTable.h"
#include "../Network/Publish.h"
#include "../MultiLayersNW/Publish.h"
#include "../../SimulateNavigate/Network/SingleLayerNW.h"
#include "../../SimulateNavigate/Network/MultiLayersNW.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::LookUpTable;

const std::wstring Publish::FILE_NAME= L"LookUpTable";

Publish::Publish(Tools::PropertySet& ps)
{
	Tools::Variant var;

	var = ps.getProperty("FilePath");
	assert (var.m_varType != Tools::VT_EMPTY);
	if (var.m_varType != Tools::VT_PWCHAR)
		throw Tools::IllegalArgumentException(
		"StorageManager: Property FilePath must be Tools::VT_PWCHAR"
		);
	std::wstring tablePath = std::wstring(var.m_val.pwcVal) + L"\\" + FILE_NAME,
		nwPath = std::wstring(var.m_val.pwcVal) + L"\\" + NVDataPublish::Network::Publish::FILE_NAME;
	m_tableFile = StorageManager::createNewStaticDiskStorageManager(tablePath, PAGE_SIZE);
	m_nwFile = NVDataPublish::Network::Publish::loadNWStorageManager(nwPath);

	m_tableBuffer = StorageManager::createNewFIFOBuffer(*m_tableFile, 1000, false);
	m_nwBuffer = StorageManager::createNewFIFOBuffer(*m_nwFile, 1000, false);

	var = ps.getProperty("MaxNodeNumPerLookUpTable");
	if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 2)
		throw Tools::IllegalArgumentException(
		"LookUpTable::Publish: Property MaxNodeNumPerLookUpTable must be Tools::VT_ULONG and >= 2"
		);
	m_capacity = var.m_val.lVal;
}

Publish::~Publish()
{
	delete m_lookUp;
	delete m_tableBuffer;
	delete m_tableFile;
	delete m_nwBuffer;
	delete m_nwFile;
}

void Publish::createRegularTable(HWND hwnd)
{
	SimulateNavigate::Network::SingleLayerNW* nw
		= new SimulateNavigate::Network::SingleLayerNW(m_nwBuffer);
	m_lookUp = new RegularGridTable(*m_tableBuffer, nw->getTopoNodeSize(), 
		m_capacity, nw->getTotScope());
	TableBuilding tb(m_lookUp);
	size_t nextPage;
	m_lookUp->outputIndexInfo(nextPage);
	tb.build(hwnd, m_nwBuffer, m_tableBuffer, nextPage);
	delete nw;
}

void Publish::createQuadTable(HWND hwnd)
{
	SimulateNavigate::Network::SingleLayerNW* nw
		= new SimulateNavigate::Network::SingleLayerNW(m_nwBuffer);
	m_lookUp = new QuadTreeTable(*m_tableBuffer, *m_nwBuffer, m_capacity, nw->getTotScope());
	TableBuilding tb(m_lookUp);
	size_t nextPage;
	m_lookUp->outputIndexInfo(nextPage);
	tb.build(hwnd, m_nwBuffer, m_tableBuffer, nextPage);
	delete nw;
}