#ifndef _IS_SDE_SPATIALINDEX_UTILITY_H_
#define _IS_SDE_SPATIALINDEX_UTILITY_H_
#include "../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			
		}

		const size_t MAX_DISPLAY_NUM = 1000;

		enum RangeQueryType
		{
			ContainmentQuery	= 0x1,
			IntersectionQuery	= 0x2,
		};
	}
}

#endif // _IS_SDE_SPATIALINDEX_UTILITY_H_
