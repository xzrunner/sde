#ifndef _IS_SDE_GENERALIZATION_MEASURE_ALL_CATEGORIES_NUMBER_H_
#define _IS_SDE_GENERALIZATION_MEASURE_ALL_CATEGORIES_NUMBER_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Measure
		{
			namespace All
			{
				class CategoriesNumber : public IMeasure
				{
				public:
					virtual void getMeasureValue(IAgent* agent, Tools::PropertySet& ps);
					// String					Value		Description
					// ------------------------------------------------------------
					// CategoriesNumber			VT_LONG		The number of shapes.

				}; // GeoScope
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_MEASURE_ALL_CATEGORIES_NUMBER_H_