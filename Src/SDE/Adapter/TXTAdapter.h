#ifndef _IS_SDE_ADAPTER_TXT_ADAPTER_H_
#define _IS_SDE_ADAPTER_TXT_ADAPTER_H_
#include "../Tools/SpatialDataBase.h"
#include "../SpatialIndex/Utility/Data.h"

namespace IS_SDE
{
	namespace Adapter
	{
		class TXT_IO
		{
		public:
			static void transFstreamToData(std::ifstream& finTxt, SpatialIndex::Data** d, id_type id);

		}; // TXT_IO
	}
}

#endif // _IS_SDE_ADAPTER_TXT_ADAPTER_H_