#ifndef IS_GIS_CRT_AFFINE_H_
#define IS_GIS_CRT_AFFINE_H_

#include "CRT.h"
#include "../Pack/Packable.h"

namespace IS_GIS
{
;

class CRT_Affine : public CRT, public Packable
{
public:
	CRT_Affine();
	CRT_Affine(const CRT_Affine &right);
	~CRT_Affine(){}

	virtual bool operator == (const CRT &crt) const;
	virtual CRT *copy() const;
	virtual bool isInversable() const;
	virtual CRT *createInverseCRT() const;

	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const;

	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
public:
	double a, b, c, d, e, f;
};

}//IS_GIS

#endif //IS_GIS_CRT_AFFINE_H_