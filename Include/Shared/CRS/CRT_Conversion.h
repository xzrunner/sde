#ifndef IS_GIS_CRT_CONVERSION_H_
#define IS_GIS_CRT_CONVERSION_H_

#include "CRT.h"
#include "Datum.h"
#include "ComputeEquation.h"

namespace IS_GIS
{
;

// from Geographic to Geocentric or inverse
class CRT_Conversion : public CRT
{
public:
	CRT_Conversion();
	CRT_Conversion(const CRT_Conversion &right)
	{
		_ellipsoid = right._ellipsoid;
		m_is_geocentric_to_geographic = right.m_is_geocentric_to_geographic;
	}
	~CRT_Conversion( )
	{ 	
	}

	IS_GIS::Pos3D convertFromGeographicToGeocentric( const IS_GIS::Pos3D& posSource) const;   //将经纬度坐标转换为空间直角坐标

	IS_GIS::Pos3D convertFromGeocentricToGeographic( const IS_GIS::Pos3D& posSource) const;   //将空间直角坐标转换为经纬度坐标

	void setGeocentricEllipsoid(CD_Ellipsoid& ellipsoid)
	{
		_ellipsoid = ellipsoid;
	}

	CD_Ellipsoid getGeocentricEllipsoid()
	{
		return _ellipsoid;
	}
	virtual bool operator == (const CRT &right) const
	{
		const CRT_Conversion * _right = dynamic_cast<const CRT_Conversion *>(&right);
		if(!_right)
			return false;
		return _ellipsoid == _right->_ellipsoid;
	}

	virtual CRT* copy() const;

	virtual bool isInversable() const
	{
		return true; 
	}

	virtual CRT *createInverseCRT() const
	{
		CRT_Conversion *ret = new CRT_Conversion(*this);
		ret->m_is_geocentric_to_geographic = ! ret->m_is_geocentric_to_geographic;
		return ret;
	}

	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const;
protected:
	//std::string _crsGeocentricName;
	CD_Ellipsoid _ellipsoid;
public:
	bool m_is_geocentric_to_geographic;
};

}//namespace

#endif//IS_GIS_CRT_CONVERSION_H_