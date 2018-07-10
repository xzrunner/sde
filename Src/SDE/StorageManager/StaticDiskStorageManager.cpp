#include "StaticDiskStorageManager.h"

using namespace IS_SDE;
using namespace IS_SDE::StorageManager;

IStorageManager* StorageManager::returnStaticDiskStorageManager(Tools::PropertySet& ps)
{
	IStorageManager* sm = new StaticDiskStorageManager(ps);
	return sm;
}

IStorageManager* StorageManager::createNewStaticDiskStorageManager(const std::wstring& baseName, size_t pageSize)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = true;
	ps.setProperty("Overwrite", var);
	// overwrite the file if it exists.

	var.m_varType = Tools::VT_PWCHAR;
	var.m_val.pwcVal = const_cast<wchar_t*>(baseName.c_str());
	ps.setProperty("FileName", var);
	// .idx and .dat extensions will be added.

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = pageSize;
	ps.setProperty("PageSize", var);
	// specify the page size. Since the index may also contain user defined data
	// there is no way to know how big a single node may become. The storage manager
	// will use multiple pages per node if needed. Off course this will slow down performance.

	return returnStaticDiskStorageManager(ps);
}

IStorageManager* StorageManager::loadStaticDiskStorageManager(const std::wstring& baseName)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_PWCHAR;
	var.m_val.pwcVal = const_cast<wchar_t*>(baseName.c_str());
	ps.setProperty("FileName", var);
	// .idx and .dat extensions will be added.

	return returnStaticDiskStorageManager(ps);
}

StaticDiskStorageManager::StaticDiskStorageManager(Tools::PropertySet& ps)
	: m_pageSize(0), m_nextPage(-1), m_buffer(NULL)
{
	Tools::Variant var;

	// Open/Create flag.
	bool bOverwrite = false;
	var = ps.getProperty("Overwrite");

	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL)
			throw Tools::IllegalArgumentException("SpatialDataBase::DiskStorageManager: Property Overwrite must be Tools::VT_BOOL");
		bOverwrite = var.m_val.blVal;
	}

	// storage filename.
	var = ps.getProperty("FileName");

	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_PWCHAR)
			throw Tools::IllegalArgumentException(
			"StorageManager: Property FilePath must be Tools::VT_PWCHAR"
			);

		std::wstring sIndexFile = std::wstring(var.m_val.pwcVal) + L".idx";
		std::wstring sDataFile = std::wstring(var.m_val.pwcVal) + L".dat";

		// check if file exists.
		bool bFileExists = true;
		std::ifstream fin1(sIndexFile.c_str(), std::ios::in | std::ios::binary);
		std::ifstream fin2(sDataFile.c_str(), std::ios::in | std::ios::binary);
		if (fin1.fail() || fin2.fail()) 
			bFileExists = false;
		fin1.close(); fin2.close();

		// check if file can be read/written.
		if (bFileExists == true && bOverwrite == false)
		{
			m_indexFile.open(sIndexFile.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			m_dataFile.open(sDataFile.c_str(), std::ios::in | std::ios::out | std::ios::binary);

			if (m_indexFile.fail() || m_dataFile.fail())
				throw Tools::IllegalArgumentException(
				"StorageManager: Index/Data file cannot be read/writen."
				);
		}
		else
		{
			m_indexFile.open(sIndexFile.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
			m_dataFile.open(sDataFile.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

			if (m_indexFile.fail() || m_dataFile.fail())
				throw Tools::IllegalArgumentException(
				"StorageManager: Index/Data file cannot be created."
				);
		}
	}
	else
	{
		throw Tools::IllegalArgumentException(
			"StorageManager: Property FileName was not specified."
			);
	}

	if (bOverwrite == true)
	{
		var = ps.getProperty("PageSize");

		if (var.m_varType != Tools::VT_EMPTY)
		{
			if (var.m_varType != Tools::VT_ULONG)
				throw Tools::IllegalArgumentException(
				"StorageManager: Property PageSize must be Tools::VT_LONG"
				);
			m_pageSize = var.m_val.ulVal;
			m_nextPage = 0;
		}
		else
		{
			throw Tools::IllegalArgumentException(
				"StorageManager: A new storage manager is created and property PageSize was not specified."
				);
		}
	}
	else
	{
		m_indexFile.read(reinterpret_cast<char*>(&m_pageSize), sizeof(size_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException(
			"StorageManager: Failed reading pageSize."
			);

		m_indexFile.read(reinterpret_cast<char*>(&m_nextPage), sizeof(id_type));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException(
			"StorageManager: Failed reading nextPage."
			);
	}

	// create buffer.
	m_buffer = new byte[m_pageSize];
	bzero(m_buffer, m_pageSize);
}

StaticDiskStorageManager::~StaticDiskStorageManager()
{
	flush();
	m_indexFile.close();
	m_dataFile.close();
	if (m_buffer != 0) 
		delete[] m_buffer;
}

void StaticDiskStorageManager::flush()
{
	m_indexFile.seekp(0, std::ios_base::beg);
	if (m_indexFile.fail())
		throw Tools::IllegalStateException(
		"DiskStorageManager::flush: Corrupted storage manager index file."
		);

	m_indexFile.write(reinterpret_cast<const char*>(&m_pageSize), sizeof(size_t));
	if (m_indexFile.fail())
		throw Tools::IllegalStateException(
		"DiskStorageManager::flush: Corrupted storage manager index file."
		);

	m_indexFile.write(reinterpret_cast<const char*>(&m_nextPage), sizeof(id_type));
	if (m_indexFile.fail())
		throw Tools::IllegalStateException(
		"DiskStorageManager::flush: Corrupted storage manager index file."
		);

	m_indexFile.flush();
	m_dataFile.flush();
}

void StaticDiskStorageManager::loadByteArray(const id_type id, size_t& len, byte** data)
{
	len = m_pageSize;
	*data = new byte[len];

	m_dataFile.seekg(id * m_pageSize, std::ios_base::beg);
	if (m_dataFile.fail())
		throw Tools::IllegalStateException(
		"StorageManager::loadByteArray: Corrupted data file."
		);

	m_dataFile.read(reinterpret_cast<char*>(m_buffer), m_pageSize);
	if (m_dataFile.fail())
		throw Tools::IllegalStateException(
		"StorageManager::loadByteArray: Corrupted data file."
		);
	memcpy(*data, m_buffer, len);
}

void StaticDiskStorageManager::storeByteArray(id_type& id, const size_t len, const byte* const data)
{
	assert(len <= m_pageSize);

	if (id == NewPage)
	{
		if (m_emptyPages.empty())
			id = m_nextPage++;
		else
		{
			id = m_emptyPages.front(); m_emptyPages.pop();
		}
	}

	m_dataFile.seekp(id * m_pageSize, std::ios_base::beg);
	if (m_dataFile.fail())
		throw Tools::IllegalStateException(
		"StorageManager::storeByteArray: Corrupted data file."
		);
	
	memcpy(m_buffer, data, len);
	m_dataFile.write(reinterpret_cast<const char*>(m_buffer), m_pageSize);
	if (m_dataFile.fail())
		throw Tools::IllegalStateException(
		"StorageManager::storeByteArray: Corrupted data file."
		);
}

void StaticDiskStorageManager::deleteByteArray(const id_type id)
{
	m_emptyPages.push(id);
}
