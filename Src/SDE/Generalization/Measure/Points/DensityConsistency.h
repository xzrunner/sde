#ifndef _IS_SDE_GENERALIZATION_MEASURE_POINTS_DENSITY_CONSISTENCY_H_
#define _IS_SDE_GENERALIZATION_MEASURE_POINTS_DENSITY_CONSISTENCY_H_
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
				class DensityConsistency : public IMeasure
				{
				public:
					DensityConsistency(IAgent* agent);

					virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps);
					// String					Value		Description
					// ------------------------------------------------------------
					// DensityConsistency		VT_DOUBLE	The standard deviation of face change.

				private:
					void fetchAreaList(IAgent* agent, std::vector<double>& areaList);

					double calDensityConsistencyVal(IAgent* agent);
					double calStandardDeviation(const std::vector<double>& val) const;

				private:
					double m_result;
					std::vector<double> m_srcAreaList;

				}; // DensityConsistency
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_MEASURE_POINTS_DENSITY_CONSISTENCY_H_
