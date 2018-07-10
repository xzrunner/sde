#ifndef _IS_SDE_GENERALIZATION_UTILITY_GENERALIZATION_MATH_H_
#define _IS_SDE_GENERALIZATION_UTILITY_GENERALIZATION_MATH_H_
#include "../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Utility
		{
			class GeneralizationMath
			{
			public:
				// Using principle of selection, equal map density
				// scaleTimes: ratio of source map's scale denominator to target map's.
				static size_t calTargetMapSymbolNum(size_t sourceNum, double scaleTimes);

			}; // GeneralizationMath
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_UTILITY_GENERALIZATION_MATH_H_
