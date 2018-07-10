#ifndef _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_UTILITY_H_
#define _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_UTILITY_H_
#include "../../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class CondenseData;
		}
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			enum RangeQueryType
			{
				ContainmentQuery	= 0x1,
				IntersectionQuery	= 0x2,
			};

			namespace StaticRTree
			{
				class StaticRTree;

				enum BulkLoadMethod
				{
					BLM_STR = 0x0
				};

				const size_t NODE_CAPACITY = 151;		// (4096 - 2 - 16) / 27

				extern ISpatialIndex* returnStaticRTree(IStorageManager& sm, Tools::PropertySet& ps);
				extern ISpatialIndex* createNewStaticRTree(
					IStorageManager& sm, 
					id_type& indexIdentifier,
					size_t condenseStrategy
					);
				extern ISpatialIndex* createAndBulkLoadNewStaticRTree(
					BulkLoadMethod m,
					IDataStream& stream,
					IStorageManager& sm,
					NVDataPublish::Features::CondenseData& cd,
					id_type& indexIdentifier,
					size_t condenseStrategy
					);
				extern ISpatialIndex* loadStaticRTree(IStorageManager& in, id_type indexIdentifier);
			}
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_UTILITY_H_