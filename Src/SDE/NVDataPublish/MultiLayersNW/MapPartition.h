#ifndef _IS_SDE_NVDATAPUBLISH_MLNW_PARTITION_MAP_H_
#define _IS_SDE_NVDATAPUBLISH_MLNW_PARTITION_MAP_H_
#include "../Base.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SingleLayerNW;
		}
	}

	namespace NVDataPublish
	{
		namespace MLNW
		{
			class Node;

			class PartitionMap
			{
			public:
				PartitionMap(IStorageManager& sm, size_t capacity);
				~PartitionMap();

				void setAllGridsEnterNodeInfo(IStorageManager& sm);

				void getAllGridsByPostorderTraversal(std::vector<Node*>& grids);

				const Node* getRoot() const { return m_root; }

				// Scope: double * 4
				// Pre-Order Travel all Nodes
				// 1bit per Node
				// 0 is Leaf, 1 is Index
				void storeToByteArray(byte** data, size_t& length) const;

			private:
				void insertEnterGridItem(const MapPos2D& s, const MapPos2D& e, size_t eID);

			private:
				SimulateNavigate::Network::SingleLayerNW* m_singleNW;

				Node* m_root;
				Rect m_scope;

			}; // PartitionMap
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_MLNW_PARTITION_MAP_H_
