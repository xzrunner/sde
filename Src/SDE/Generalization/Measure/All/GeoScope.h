#ifndef _IS_SDE_GENERALIZATION_MEASURE_ALL_GEO_SCOPE_H_
#define _IS_SDE_GENERALIZATION_MEASURE_ALL_GEO_SCOPE_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Measure
		{
			namespace All
			{
				class GeoScope : public IMeasure
				{
				public:
					virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps);
					// String					Value		Description
					// ------------------------------------------------------------
					// GeoScope					VT_PVOID	MBR. [MapScope*]

				}; // GeoScope
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_MEASURE_ALL_GEO_SCOPE_H_