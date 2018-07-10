#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_ENTER_GRID_NODES_MAPPING_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_ENTER_GRID_NODES_MAPPING_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace MLNW
		{
			class Node;
			class EnterGridNodesMapping
			{
			public:
				EnterGridNodesMapping(const Node* root);
				~EnterGridNodesMapping();

				void outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const;

			public:
				typedef size_t SINGLE_NODE_ID;
				typedef size_t MULTI_NODE_ID;
				typedef size_t LAYER;
				typedef size_t ADDRESS;
				typedef size_t OFFSET;

				class Entry
				{
				public:
					typedef std::map<LAYER, MULTI_NODE_ID> MAP_LAYER_ID;

				public:
					Entry(size_t singleID) : m_singleID(singleID) {}

					void insert(size_t layer, size_t multiID);

					bool beLegal() const { 
						return m_data.size() == 1 || m_data.size() > 1 && 
							((--m_data.end())->first - m_data.begin()->first == m_data.size() - 1);
					}

					size_t dataStoreSize() const { 
						return START_LAYER_SIZE + MULTI_NODE_ID_SIZE * m_data.size(); 
					}

					size_t singleID() const { return m_singleID; }

					const MAP_LAYER_ID& dataRef() const { return m_data; }

				public:
					static const size_t START_LAYER_SIZE	= 2;
					static const size_t MULTI_NODE_ID_SIZE	= sizeof(MULTI_NODE_ID);

				private:
					size_t m_singleID;

					MAP_LAYER_ID m_data;

				}; // Entry

				class Layer2IndexStorage;
				class Layer1IndexStorage
				{
				public:
					void prepareData(const Layer2IndexStorage& index2);
					void outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const;

				public:
					static const size_t PAGE_COUNT	= 1;
					static const size_t ITEM_SIZE	= sizeof(SINGLE_NODE_ID);
					static const size_t END_FLAG	= 0xffffffff;

				private:
					std::vector<SINGLE_NODE_ID> m_data;

				}; // Layer1IndexStorage

				class EntryDataStorage;
				class Layer2IndexStorage
				{
				public:
					typedef std::pair<SINGLE_NODE_ID, ADDRESS>	LAYER2_INDEX_ITEM;
					typedef std::vector<LAYER2_INDEX_ITEM>		PAGE;
					typedef std::vector<PAGE>					PAGES;

				public:
					void prepareData(size_t totDataCount, const EntryDataStorage& entryData, 
						size_t pageSize, id_type nextPage);
					void outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const;

					const PAGES& dataRef() const { return m_data; }

				public:
					static const size_t LAYER2_INDEX_ITEM_SIZE = sizeof(SINGLE_NODE_ID) + sizeof(ADDRESS);

				private:
					PAGES m_data;

				}; // Layer2IndexStorage

				class EntryDataStorage
				{
				public:
					typedef std::pair<Entry*, OFFSET>		PAIR_ENTRY_ADDR;
					typedef std::vector<PAIR_ENTRY_ADDR>	PAGE;
					typedef std::vector<PAGE>				PAGES;

				public:
					void prepareData(const std::map<SINGLE_NODE_ID, Entry*>& srcData, size_t pageSize);
					void outputToStorage(IStorageManager& sm, size_t pageSize, id_type& nextPage) const;

					const PAGES& dataRef() const { return m_data; }

				private:
					PAGES m_data;

				}; // EntryDataStorage

			private:
				void insert(size_t singleID, size_t layer, size_t multiID);

			private:
				std::map<SINGLE_NODE_ID, Entry*> m_data;

			}; // EnterGridNodesMapping
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_ENTER_GRID_NODES_MAPPING_H_
