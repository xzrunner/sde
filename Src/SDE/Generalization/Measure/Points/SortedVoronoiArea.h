#ifndef _IS_SDE_GENERALIZATION_MEASURE_POINTS_SORTED_VORONOI_AREA_H_
#define _IS_SDE_GENERALIZATION_MEASURE_POINTS_SORTED_VORONOI_AREA_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Algorithms
	{
		class VoronoiByDelaunay;
	}

	namespace Generalization
	{
		namespace Measure
		{
			namespace Points
			{
				class SortedVoronoiArea : public IMeasure
				{
				public:
					virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps);
					// String					Value		Description
					// ------------------------------------------------------------
					// SortedVoronoiArea		VT_PVOID	The area list of all points. [(size_t + std::vector<double>)*]

					// Points Agent
					static void calSortedVoronoiArea(Algorithms::VoronoiByDelaunay* voronoi, std::vector<double>& areaList);

				}; // SortedVoronoiArea
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_MEASURE_POINTS_SORTED_VORONOI_AREA_H_