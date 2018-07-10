#ifndef _IS_SDE_SPATIALINDEX_RTREE_UTILITY_H_
#define _IS_SDE_SPATIALINDEX_RTREE_UTILITY_H_
#include "../Utility/Utility.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace RTree
		{
			class RTree;

			enum RTreeVariant
			{
				RV_LINEAR = 0x0,
				RV_QUADRATIC,
				RV_RSTAR
			};

			enum BulkLoadMethod
			{
				BLM_STR = 0x0
			};

			enum PersistenObjectIdentifier
			{
				PersistentIndex = 0x1,
				PersistentLeaf	= 0x2
			};

			extern ISpatialIndex* returnRTree(IStorageManager& sm, Tools::PropertySet& ps);
			extern ISpatialIndex* createNewRTree(
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				RTreeVariant rv,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* createAndBulkLoadNewRTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t indexCapacity,
				size_t leafCapacity,
				RTreeVariant rv,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* loadRTree(IStorageManager& in, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_UTILITY_H_
