#ifndef IS_GIS_CRS_PACKER_H_
#define IS_GIS_CRS_PACKER_H_

#include "CRS.h"

namespace IS_GIS
{

class CRS_packer
{
public:

	static int pack(const CRS *from, byte_8 **result, uint_32 *len);

};

}//IS_GIS

#endif//IS_GIS_CRS_PACKER_H_