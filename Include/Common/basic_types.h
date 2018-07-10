// basic_types.h
// zzj, Jan. 13
// define the basic data types such as integers
#ifndef IS_GIS_BASIC_TYPES_H_
#define IS_GIS_BASIC_TYPES_H_

#ifndef WIN32
#define WIN32
#endif
#include "common_config.h"
 
namespace IS_GIS
{

#ifdef WIN32

#define int_32 int
#define uint_32 unsigned int
#define int_64 __int64
#define uint_64 unsigned __int64
#define int_16 short
#define uint_16 unsigned short
#define bool_32 int

#endif	//WIN32

#define byte_8 unsigned char


}//namespace IS_GIS

#endif //IS_GIS_BASIC_TYPES_H_

