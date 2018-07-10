#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MATH_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MATH_H_
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Math
			{
			public:
				static bool isPosHigher(const MapPos2D& lhs, const MapPos2D& rhs) {
					return lhs.d1 > rhs.d1
						|| lhs.d1 == rhs.d1 && lhs.d0 < rhs.d0;
				}

				static bool isPosLefter(const MapPos2D& lhs, const MapPos2D& rhs) {
					return lhs.d0 < rhs.d0
						|| lhs.d0 == rhs.d0 && lhs.d1 > rhs.d1;
				}

			}; // Math
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MATH_H_