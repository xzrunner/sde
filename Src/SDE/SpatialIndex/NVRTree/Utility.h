#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_UTILITY_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_UTILITY_H_
#include "../Utility/Utility.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class Node;
			typedef Tools::PoolPointer<Node> NodePtr;

			class NVRTree;

			enum BulkLoadMethod
			{
				BLM_STR = 0x0
			};

			enum PersistenObjectIdentifier
			{
				PersistentIndex = 0x1,
				PersistentLeaf	= 0x2
			};

			extern ISpatialIndex* returnNVRTree(IStorageManager& sm, Tools::PropertySet& ps);
			extern ISpatialIndex* createNewNVRTree(
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* createAndBulkLoadNewNVRTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* loadNVRTree(IStorageManager& in, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_UTILITY_H_
