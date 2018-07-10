// System.h

#ifndef IS_GIS_SYSTEM_H_
#define IS_GIS_SYSTEM_H_

#include "../Common/basic_types.h"

const byte_8 BIG_ENDIAN_TAG = 0x00;
const byte_8 LIT_ENDIAN_TAG = 0xff;


#ifdef WIN32
const byte_8 SYS_ENDIAN_TAG = LIT_ENDIAN_TAG;
#endif //#ifdef WIN32

#endif //#ifndef IS_GIS_SYSTEM_H_