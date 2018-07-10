#include "EnterGridNodesMapping.h"
#include "Node.h"
#include "Index.h"
#include "../Network/AdjacencyList.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::MLNW;

typedef NVDataPublish::Network::AdjacencyList AL;

EnterGridNodesMapping::EnterGridNodesMapping(const Node* root)
{
	std::queue<std::pair<Node*, LAYER> > buffer;
	buffer.push(std::make_pair(const_cast<Node*>(root), 0));
	while (!buffer.empty())
	{
		std::pair<Node*, LAYER> pairGridLayer = buffer.front(); buffer.pop();
		std::map<SINGLE_NODE_ID, MULTI_NODE_ID>::iterator itrSingleToMulti
			= pairGridLayer.first->m_transEnterGridNodeIDs.begin();
		for ( ; itrSingleToMulti != pairGridLayer.first->m_transEnterGridNodeIDs.end(); ++itrSingleToMulti)
		{
			if (itrSingleToMulti->second != 0)
				insert(itrSingleToMulti->first, pairGridLayer.second, itrSingleToMulti->second);
		}

		if (!pairGridLayer.first->isLeaf())
		{
			Index* index = dynamic_cast<Index*>(pairGridLayer.first);
			for (size_t i = 0; i < 4; ++i)
				buffer.push(std::make_pair(index->m_child[i], pairGridLayer.second + 1));
		}
	}
}

EnterGridNodesMapping::~EnterGridNodesMapping()
{
	std::map<SINGLE_NODE_ID, Entry*>::iterator itr = m_data.begin();
	for ( ; itr != m_data.end(); ++itr)
		delete itr->second;
}

void EnterGridNodesMapping::outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const
{
	EntryDataStorage entryData;
	entryData.prepareData(m_data, pageSize);

	Layer2IndexStorage index2;
	index2.prepareData(m_data.size(), entryData, pageSize, nextPage);

	Layer1IndexStorage index1;
	index1.prepareData(index2);

	index1.outputToStorage(sm, pageSize, nextPage);
	index2.outputToStorage(sm, pageSize, nextPage);
	entryData.outputToStorage(sm, pageSize, nextPage);
}

//
// class EnterGridNodesMapping::Entry
//

void EnterGridNodesMapping::Entry::
	insert(size_t layer, size_t multiID)
{
	assert(m_data.find(layer) == m_data.end());
	m_data.insert(std::make_pair(layer, multiID));
}

//
// class EnterGridNodesMapping::Layer1IndexStorage
//

void EnterGridNodesMapping::Layer1IndexStorage::
	prepareData(const Layer2IndexStorage& index2)
{
	Layer2IndexStorage::PAGES::const_iterator itr = index2.dataRef().begin(),
		itrEnd = index2.dataRef().end();
	for ( ; itr != itrEnd; ++itr)
		m_data.push_back((itr->back()).first);
}

void EnterGridNodesMapping::Layer1IndexStorage::
	outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const
{
	byte* data = new byte[pageSize];
	byte* ptr = data;

	size_t itemCount = m_data.size();
	assert(ITEM_SIZE * itemCount <= pageSize);
	memset(data, 0xff, pageSize);
	for (size_t i = 0; i < itemCount; ++i)
	{
		memcpy(ptr, &m_data[i], ITEM_SIZE);
		ptr += ITEM_SIZE;
	}

	sm.storeByteArray(nextPage, pageSize, data);
	++nextPage;

	delete[] data;
}

//
// class EnterGridNodesMapping::Layer2IndexStorage
//

void EnterGridNodesMapping::Layer2IndexStorage::
	prepareData(size_t totDataCount, const EntryDataStorage& entryData, size_t pageSize, id_type nextPage)
{
	size_t layer2IndexSize = totDataCount * LAYER2_INDEX_ITEM_SIZE;
	assert(pageSize % LAYER2_INDEX_ITEM_SIZE == 0);
	size_t layer2IndexPageCount = static_cast<size_t>( std::ceil(
		static_cast<double>(layer2IndexSize) / static_cast<double>(pageSize)
		) );
	size_t dataPageStart 
		= static_cast<size_t>(nextPage) 
		+ Layer1IndexStorage::PAGE_COUNT 
		+ layer2IndexPageCount;

	size_t remain = pageSize;
	PAGE layer2IndexPage;

	//// debug
	//std::ofstream fout("c:/3_23_pre.txt");
	//fout << "Prepare: " << std::endl;
	////
	EntryDataStorage::PAGES::const_iterator itrDataPage = entryData.dataRef().begin(), 
		itrDataPageEnd = entryData.dataRef().end();
	for ( ; itrDataPage != itrDataPageEnd; ++itrDataPage)
	{
		EntryDataStorage::PAGE::const_iterator itrDataEntry = (*itrDataPage).begin(),
			itrDataEntryEnd = (*itrDataPage).end();
		for ( ; itrDataEntry != itrDataEntryEnd; ++itrDataEntry)
		{
			SINGLE_NODE_ID singleID = itrDataEntry->first->singleID();
			if (remain < LAYER2_INDEX_ITEM_SIZE)
			{
				m_data.push_back(layer2IndexPage);
				layer2IndexPage.clear();
				remain = pageSize - LAYER2_INDEX_ITEM_SIZE;
			}
			else
				remain -= LAYER2_INDEX_ITEM_SIZE;
			//// debug
			//fout << dataPageStart << '\t' << itrDataEntry->second << std::endl;
			////
			ADDRESS addr = AL::encodeTopoNodeID(dataPageStart, itrDataEntry->second);
			layer2IndexPage.push_back(std::make_pair(singleID, addr));
		}
		++dataPageStart;
	}

	if (!layer2IndexPage.empty())
		m_data.push_back(layer2IndexPage);
}

void EnterGridNodesMapping::Layer2IndexStorage::
	outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const
{
	byte* data = new byte[pageSize];
	byte* ptr = data;

	//// debug
	//std::ofstream fout("c:/3_23_index2.txt");
	//fout << "Index2: " << std::endl;
	////

	PAGES::const_iterator itrPage = m_data.begin(), itrPageEnd = m_data.end();
	for ( ; itrPage != itrPageEnd; ++itrPage)
	{
		memset(data, 0xff, pageSize);

		PAGE::const_iterator itrEntry = itrPage->begin(), itrEntryEnd = itrPage->end();
		for ( ; itrEntry != itrEntryEnd; ++itrEntry)
		{
			//// debug
			//fout << nextPage << '\t' << ptr - data << '\t' << itrEntry->first << std::endl;
			////

			memcpy(ptr, &itrEntry->first, sizeof(SINGLE_NODE_ID));
			ptr += sizeof(SINGLE_NODE_ID);
			memcpy(ptr, &itrEntry->second, sizeof(ADDRESS));
			ptr += sizeof(ADDRESS);
		}

		sm.storeByteArray(nextPage, pageSize, data);
		++nextPage;
		ptr = data;
	}

	delete[] data;
}

//
// class EnterGridNodesMapping::EntryDataStorage
//

void EnterGridNodesMapping::EntryDataStorage::
	prepareData(const std::map<SINGLE_NODE_ID, Entry*>& srcData, size_t pageSize)
{
	size_t offset = 0;
	PAGE dataPage;

	std::map<SINGLE_NODE_ID, Entry*>::const_iterator itr = srcData.begin();
	for ( ; itr != srcData.end(); ++itr)
	{
		Entry* e = itr->second;
		assert(e->beLegal());
		size_t entrySize = e->dataStoreSize();
		if (entrySize < pageSize - offset)
		{
			dataPage.push_back(std::make_pair(e, offset));
			offset += entrySize;
		}
		else
		{
			assert(itr != srcData.begin());
			m_data.push_back(dataPage);
			dataPage.clear();
			dataPage.push_back(std::make_pair(e, 0));
			offset = entrySize;
		}
	}

	if (!dataPage.empty())
		m_data.push_back(dataPage);
}

void EnterGridNodesMapping::EntryDataStorage::
	outputToStorage(IS_SDE::IStorageManager& sm, size_t pageSize, IS_SDE::id_type &nextPage) const
{
	byte* data = new byte[pageSize];
	byte* ptr = data;

	//// debug
	//std::ofstream fout("c:/3_23_output.txt");
	////

	PAGES::const_iterator itrPage = m_data.begin(), itrPageEnd = m_data.end();
	for ( ; itrPage != itrPageEnd; ++itrPage)
	{
		PAGE::const_iterator itrEntry = itrPage->begin(), itrEntryEnd = itrPage->end();
		for ( ; itrEntry != itrEntryEnd; ++itrEntry)
		{
			const Entry::MAP_LAYER_ID& items = itrEntry->first->dataRef();
			Entry::MAP_LAYER_ID::const_iterator itrItem = items.begin(), itrItemEnd = items.end();
			//// debug
			//fout << items.begin()->first << '\t' << itrItem->second << std::endl;
			////
			memcpy(ptr, &itrItem->first, Entry::START_LAYER_SIZE);
			ptr += Entry::START_LAYER_SIZE;
			for ( ; itrItem != itrItemEnd; ++itrItem)
			{
				memcpy(ptr, &itrItem->second, sizeof(MULTI_NODE_ID));
				ptr += sizeof(MULTI_NODE_ID);
			}
		}

		sm.storeByteArray(nextPage, pageSize, data);
		++nextPage;
		ptr = data;
	}

	delete[] data;
}

//
// Internal
//

void EnterGridNodesMapping::insert(size_t singleID, size_t layer, size_t multiID)
{
	std::map<SINGLE_NODE_ID, Entry*>::iterator itrEntry
		= m_data.find(singleID);
	if (itrEntry == m_data.end())
	{
		Entry* e = new Entry(singleID);
		e->insert(layer, multiID);
		m_data.insert(std::make_pair(singleID, e));
	}
	else
		itrEntry->second->insert(layer, multiID);
}
