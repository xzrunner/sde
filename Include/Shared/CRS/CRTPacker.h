#ifndef _CRT_PACKER_H_
#define _CRT_PACKER_H_

#include "CRT.h"

namespace IS_GIS
{
	class CRTPacker
	{
	public:
		static int packer(const CRT* from,byte_8** result,uint_32* len);
		static int unpacker(const byte_8* from,CRT** crt,uint_32* used_len, uint_32 buf_len = PACK_IGNORE_BUF_SIZE);
	};
}

#endif