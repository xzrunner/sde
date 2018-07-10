#ifndef _IS_SDE_ALGORITHMS_CONVEX_HULL_H_
#define _IS_SDE_ALGORITHMS_CONVEX_HULL_H_
#include "../../Tools/Tools.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace Algorithms
	{
		class ConvexHull
		{
		public:
			static void getConvexHullPos(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest);

		private:


		}; // ConvexHull
	}
}

#endif // _IS_SDE_ALGORITHMS_CONVEX_HULL_H_
