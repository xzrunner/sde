#ifndef IS_GIS_CRS_UNPACKER_H_
#define IS_GIS_CRS_UNPACKER_H_

#include "../Pack/Packable.h"
#include "CRS.h"

namespace IS_GIS
{
;

class CRS_Unpacker
{
public:
	static int_32 create(const byte_8 *from, 
		Packable **result, uint_32 *used_len, uint_32 buf_len = PACK_IGNORE_BUF_SIZE);

	static int unpack(const byte_8 *from,
		CRS **result, uint_32 *used_len, uint_32 buf_len = PACK_IGNORE_BUF_SIZE);
};

}//IS_GIS

#endif//IS_GIS_CRS_UNPACKER_H_