#ifndef IS_GIS_CRT_AFFINE2_H_
#define IS_GIS_CRT_AFFINE2_H_

#include "CRT.h"
#include "../Pack/Packable.h"

namespace IS_GIS
{
;

class CRT_QuadraticConversion : public CRT, public Packable
{
public:
	CRT_QuadraticConversion();
	CRT_QuadraticConversion(const CRT_QuadraticConversion &right);
	~CRT_QuadraticConversion(){}

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
	double a1, b1, c1, d1, e1, f1,a2,b2,c2,d2,e2,f2;
};

}//IS_GIS

#endif //IS_GIS_CRT_AFFINE2_H_