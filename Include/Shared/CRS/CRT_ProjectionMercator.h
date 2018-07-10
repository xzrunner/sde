#ifndef IS_GIS_CRT_PROJECTIONMERCATOR_H_
#define IS_GIS_CRT_PROJECTIONMERCATOR_H_

#include"CRT_Projection.h"
#include "Datum.h"
#include "../Pack/Packable.h"
namespace IS_GIS 
{
;	
	
class CRT_ProjectionMercator : public CRT_Projection, public Packable
{
public:
	CRT_ProjectionMercator();
	CRT_ProjectionMercator(const CRT_ProjectionMercator &right);
	virtual ~CRT_ProjectionMercator();
	
	virtual CRT *copy() const;
	virtual bool operator == (const CRT &right) const;
	virtual bool isInversable() const {	return true;	}
	virtual CRT *createInverseCRT() const;
	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const;

	virtual PrjType prjType() const	{	return PRJ_MERCATOR;	}

	virtual int_32 packSize(const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
	virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
	virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);
public:
	CD_Ellipsoid m_ellipsoid;
	double m_Standard_Parallel1;//�������ο��� = l*cos(m_Standard_Parallel1)
	double m_false_east;
	double m_false_north;
	double m_central_meridian;	//����������
};

class CRT_ProjectionMercator_Inverse : public CRT
{
public:
	CRT_ProjectionMercator_Inverse();
	CRT_ProjectionMercator_Inverse(const CRT_ProjectionMercator_Inverse &right);
	CRT_ProjectionMercator_Inverse(const CRT_ProjectionMercator &prj_mercator);
	virtual ~CRT_ProjectionMercator_Inverse();

	virtual CRT *copy() const;
	virtual bool operator == (const CRT &right) const;
	virtual bool isInversable() const {	return true;	}
	virtual CRT *createInverseCRT() const;
	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const;

public:
	CRT_ProjectionMercator m_prj_mercator;
};

}//namespace IS_GIS

#endif //IS_GIS_CRT_PROJECTIONMERCATOR_H_