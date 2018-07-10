// packable.h
// to define the interfaces for objects that can pack themselves
#ifndef IS_GIS_PACKABLE_H_
#define IS_GIS_PACKABLE_H_

#include <ios>
#include "../../Common/basic_types.h"
#include "../../System/system.h"

namespace IS_GIS{

const int_32 PACK_IGNORE_BUF_SIZE =	0x7fffffff;
const int_32 PACK_MAX_BUF_SIZE =	0x7ffffffe;

// this class is intended for client/server communication
class Packable
{
public:
	virtual ~Packable(){}
	// size of package
	virtual int_32 packSize(const void *how_to = NULL) const = 0;

	// buffer: [in] a pointer to the buffer where the object is to pack into
	// buffer_size: [in] the size of buffer pointed by start_pos
	// end_pos: [out] a pointer specifying the end of the package
	// return value: size of the package, equal to packSize() for a successful call
	// pack() throws exceptions if the buffer size is not enough
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const = 0;
	// allocate a buffer and pack into it, return == packSize();
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const = 0;

	// to unpack from the buffer - a factory is needed
	// this method is likely to throw an exception if the data in buffer is not accepted
	// by the object! if the buf_size is PACK_IGNORE_BUF_SIZE, this argument is ignored, 
	// we don't use 0 because a value of zero is likely to create a runtime error.
	// return: how many bytes in buffer used during the unpacking, -1 for error
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE) = 0;
};

class SegPackKey;

// a SegmentPackable Obj must be a Packable obj
class SegmentPackable : public Packable
{
public:
	// buffer: where packing to
	// buf_size: size of the buffer
	// end_pos: pointing to the end of the package after the call
	// nextKey: [out], lastKey:[in] - if the package contains only segment of the obj, 
	// use nextKey to put in the lastKey position in the next call to get the next 
	// segment package.  if the whole object is packaged, *nextKey will be NULL. a lastKey
	// of NULL indicates that the obj should package from its very beginning.
	// return: the actual size of the package
	virtual int_32 segPack(byte_8 *buffer, int_32 buf_size, byte_8 **end_pos,
		const SegPackKey **nextKey, const SegPackKey *lastKey = NULL, const void *how_to = NULL) const = 0;
	virtual int_32 segPack(int_32 buf_size, byte_8 **end_pos,
		const SegPackKey **nextKey, const SegPackKey *lastKey = NULL, const void *how_to = NULL) const = 0;

	// the unpack proceedure will be very different from relizations, each type
	// has its own behavior
	virtual int_32 segUnpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE) const = 0;
	
};

// subtype of SegmentPackable will have different subtype of SegPackKey
class SegPackKey
{
public:
	virtual ~SegPackKey(){}
	// the number of the package
	virtual int_32 number() const = 0;
};

} //namespace IS_GIS{

//---------------------------------pack macros-----------------------

#ifndef PACK_SIMPLE_MEMBER
#define PACK_SIMPLE_MEMBER(ptr, member) \
	memcpy(ptr, &member, sizeof(member));	\
	ptr += sizeof(member);
#endif //PACK_SIMPLE_MEMBER(ptr, member)

#ifndef PACK_PTR_MEMBER
#define PACK_PTR_MEMBER(target_ptr, value_ptr, size)\
	*(int_32*)ptr = size;\
	ptr += sizeof(int_32);\
	memcpy(target_ptr, value_ptr, size);\
	target_ptr += size;
#endif //PACK_PTR_MEMBER

#ifndef PACK_VECTOR
#define PACK_VECTOR(ptr, vec, val_type)\
	*(uint_32 *)ptr = vec.size();\
	ptr += sizeof(uint_32);\
	memcpy(ptr, &vec[0], sizeof(val_type) * vec.size());\
	ptr += sizeof(val_type) * vec.size();
#endif //PACK_VECTOR

#ifndef PACK_INT_VAL
#define PACK_INT_VAL(ptr, val)\
	*(int_32*)ptr = val;\
	ptr += sizeof(int_32);
#endif//PACK_INT_VAL

//#ifndef PACK_STL_VAL
//#define PACK_STL_VAL(ptr, begin, end, work, count, val_type)
//	{
//		*(uint_32*)ptr = count;
//		ptr += sizeof(uint_32);
//		for(work = begin; work !=end; ++work)
//		{
//			memcpy(
//		}
//	}
//#endif //PACK_STL_VAL

//------------------------------------ unpack macros------------------------
#ifndef UNPACK_SIMPLE_MEMBER
#define UNPACK_SIMPLE_MEMBER(ptr, val)\
	memcpy(&val, ptr, sizeof(val));\
	ptr += sizeof(val);
#endif //UNPACK_SIMPLE_MEMBER

#ifndef UNPACK_PTR_MEMBER
#define UNPACK_PTR_MEMBER(mem_ptr, src_ptr, value_type)\
	{\
		int_32 ___sz = *(int_32*)src_ptr;\
		src_ptr += sizeof(int_32);\
		mem_ptr = (value_type*)(new byte_8[___sz]);\
		memcpy(mem_ptr, ptr, ___sz );\
		src_ptr += ___sz;\
	}
#endif //UNPACK_PTR_MEMBER

#ifndef UNPACK_VECTOR
#define UNPACK_VECTOR(vec, ptr, value_type)\
	{\
		int_32 ___sz = *(int_32*)ptr;\
		ptr += sizeof(int_32);\
		vec.resize(___sz);\
		memcpy(&vec[0], ptr, ___sz * sizeof(value_type));\
		ptr += ___sz * sizeof(value_type);\
	}
#endif //UNPACK_VECTOR

#endif //#ifndef IS_GIS_PACKABLE_H_