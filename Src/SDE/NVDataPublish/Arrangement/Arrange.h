#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ARRANGE_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ARRANGE_H_
#include "../../Tools/SpatialDataBase.h"
#include <windows.h>

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Arrange
			{
			public:
				static void arrangeSrcData(
					HWND hwnd, 
					const std::vector<ISpatialIndex*>& layers, 
					Tools::PropertySet& ps
					);

			}; // Arrange
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ARRANGE_H_