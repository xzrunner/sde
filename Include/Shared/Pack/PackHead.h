#ifndef IS_GIS_PACK_HEAD_H_
#define IS_GIS_PACK_HEAD_H_

#include "../../Common/basic_types.h"
#include "../SharedType/basic_gm_type.h"
#include <assert.h>

namespace IS_GIS{

// the head structure of a package
#pragma pack(push, 1)
struct PackHead
{
	// 0x00 for big-ending, other value for small ending
	byte_8 byte_order;
	// the total size of the package
	uint_32 size;
	// type of the packed obj
	uint_32 type;
	// 0x00 for non-segment package, other value for segmented package
	byte_8 is_segment;
public:
	PackHead(byte_8 b_o = SYS_ENDIAN_TAG, 
		uint_32 sz = 0, uint_32 tp = 0, byte_8 seg = 0) 
		: byte_order(b_o), size(sz), type(tp), is_segment(seg){}
};
#pragma pack(pop)

const uint_32 PACK_HEAD_SIZE = sizeof(PackHead);

inline void SET_PACK_HEAD(PackHead *ph, uint_32 type, uint_32 size)
{
	ph->byte_order = SYS_ENDIAN_TAG;
	ph->is_segment = false;
	ph->size = size;
	ph->type = type;
}

inline gm_bool packageSizeMatch(const byte_8 * package, bool endian_same)
{
	if(endian_same)
	{
		return	((const PackHead *)package)->size ==
			*((const uint_32 *)(package + ((const PackHead *)package)->size - 4));
	}
	else
	{
		assert(0);
		// to be continued...
		return false;
	}
}


} //namespace IS_GIS{
#endif //#ifndef IS_GIS_PACK_HEAD_H_