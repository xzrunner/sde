#ifndef _IS_SDE_GENERALIZATION_MEASURE_POINTS_MINIMAL_DISTANCE_H_
#define _IS_SDE_GENERALIZATION_MEASURE_POINTS_MINIMAL_DISTANCE_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Algorithms
	{
		class DelaunayTriangulation;
	}

	namespace Generalization
	{
		namespace Measure
		{
			namespace Points
			{
				class MinimalDistance : public IMeasure
				{
				public:
					virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps);
					// String					Value		Description
					// ------------------------------------------------------------
					// MinimalDistance			VT_DOUBLE	Minimum distance between objects.
					// NearestTwoPoints			VT_PVOID	The coords of nearest two points.[MapPos2D[2]]

					// Points Agent
					static double calMinimumDistance(const Algorithms::DelaunayTriangulation* dt,
						MapPos2D& p0, MapPos2D& p1);
					static double calMinimumDistance(const std::vector<MapPos2D>& points,
						MapPos2D& p0, MapPos2D& p1);

				}; // MinimalDistance
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_MEASURE_POINTS_MINIMAL_DISTANCE_H_