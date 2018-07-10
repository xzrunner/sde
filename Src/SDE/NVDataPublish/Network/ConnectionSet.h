#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_CONNECTION_SET_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_CONNECTION_SET_H_
#include "../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class Publish;

			class ConnectionSet
			{
			public:
				static void setStrongConnectionSetID(Publish* pb, size_t endPage);

			private:
				typedef size_t PAGE;
				typedef size_t OFFSET;
				typedef std::pair<PAGE, OFFSET> ADDR;

			private:
				static bool setConnectionID(Publish* pb, std::queue<ADDR>& buffer, size_t id,
					size_t checkPage, bool& bPageDirty);

				static void test(Publish* pb, size_t endPage);


			}; // ConnectionSet
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_CONNECTION_SET_H_