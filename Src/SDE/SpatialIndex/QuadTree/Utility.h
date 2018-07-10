#ifndef _IS_SDE_SPATIALINDEX_QUAD_TREE_UTILITY_H_
#define _IS_SDE_SPATIALINDEX_QUAD_TREE_UTILITY_H_
#include "../Utility/Utility.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class QuadTree;

			const int INDEX_CHILDREN_SIZE = 4;
			const int INDEX_CHILDREN_SIZE_EACH_DIM = 2;
			const id_type MAX_ID = std::numeric_limits<id_type>::max();
			const id_type HEADER_PHYSICAL_ID = 1;

			enum NodeType
			{
				NT_NULL = 0x0,
				NT_Index,
				NT_Leaf,	
				NT_Overflow
			};

			enum IndexChildPos
			{
				QUAD_NW = 0x0,
				QUAD_NE,
				QUAD_SW,
				QUAD_SE
			};

			enum BulkLoadMethod
			{
				BLM_TYPE0 = 0x0		// init all leaves node once
									// batch insert data
									// node id code is: parent = p, 
									// QUAD_NW child = p * 4 + 1, QUAD_NE child = p * 4 + 2
									// QUAD_SW child = p * 4 + 3, QUAD_SE child = p * 4 + 4
			};

			class Utility
			{
			public:
				static id_type getParentID(id_type id) {
					if (id <= 0)
						return -1;
					else
						return static_cast<size_t>(
						static_cast<double>(id - 1) / static_cast<double>(INDEX_CHILDREN_SIZE)
						);
				}
			}; // Utility

			extern ISpatialIndex* returnQuadTree(IStorageManager& sm, Tools::PropertySet& ps);
			extern ISpatialIndex* createNewQuadTree(
				IStorageManager& sm,
				double fillFactor,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* createAndBulkLoadNewQuadTree(
				BulkLoadMethod m,
				IDataStream& stream,
				IStorageManager& sm,
				double fillFactor,
				size_t leafCapacity,
				id_type& indexIdentifier
				);
			extern ISpatialIndex* loadQuadTree(IStorageManager& in, id_type indexIdentifier);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUAD_TREE_UTILITY_H_
