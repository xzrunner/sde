#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_SPATIAL_INDEX_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_SPATIAL_INDEX_H_
#include "../NavigateBase.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class Leaf;
		}

		namespace LookUpTable
		{
			class QuadTreeTable;
		}

		namespace MLNW
		{
			class PartitionMap;
		}
	}
	
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SingleLayerNW;
			class SLSpatialIndex;

			class SNode
			{
			public:
				SNode(const Rect& scope, size_t count, bool bLeaf);
				virtual ~SNode() {}

			public:
				bool m_bLeaf;
				size_t m_topoNodeCount;
				Rect m_scope;

			}; // SNode

			class SLeaf : public SNode
			{
			public:
				SLeaf(const Rect& scope, size_t count);
				virtual ~SLeaf() {}

			public:
				size_t m_startALPage;
				size_t m_pageCount;

			}; // SLeaf

			class SIndex : public SNode
			{
			public:
				SIndex(const Rect& scope, size_t count);
				virtual ~SIndex();

			public:
				// 0 1
				// 2 3
				SNode* m_child[4];

			}; // SIndex

			class SLSpatialIndex
			{
			public:
				SLSpatialIndex(IStorageManager& sm);
				~SLSpatialIndex();

			private:
				void build();

			private:
				SimulateNavigate::Network::SingleLayerNW* m_network;

				SNode* m_root;
				const Rect& m_scope;

				typedef size_t PAGE_ID;
				typedef size_t OFFSET;
				typedef std::pair<PAGE_ID, OFFSET> ADDR;

				friend class NVDataPublish::LookUpTable::QuadTreeTable;
				friend class NVDataPublish::MLNW::PartitionMap;

			}; // SLSpatialIndex
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SINGLE_SPATIAL_INDEX_H_