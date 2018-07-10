
#ifndef IS_GIS_DATUM_H_
#define IS_GIS_DATUM_H_

#include "SpatialRefMeta.h"
#include "DatumType.h"


namespace IS_GIS{

//enum GCSType;

#pragma pack(push, 4)
struct CD_PrimeMeridian
{
	MC_Angle _greenwichLongitude; //the longitude of this prime meridian relative to the GreenWich Meridian.expressed in the related angular unit.
	
	CD_PrimeMeridian & operator = (const CD_PrimeMeridian &right)
	{
		_greenwichLongitude = right._greenwichLongitude;

		return *this;
	}

	bool operator == ( const CD_PrimeMeridian & right ) const
	{
		return ( _greenwichLongitude == right._greenwichLongitude); 
	}
};

struct CD_Ellipsoid
{
	//std::string _strName;
	MC_Length _semiMajorAxis;
	MC_Length _semiMinorAxis;
	double  _dPowFlattening;  //f = (a^2-b^2)/a^2

	CD_Ellipsoid & operator = (const CD_Ellipsoid &right)
	{
		//_strName = right._strName;
		_semiMajorAxis = right._semiMajorAxis;
		_semiMinorAxis = right._semiMinorAxis;
		_dPowFlattening = right._dPowFlattening;

		return *this;
	}
	
	bool operator == ( const CD_Ellipsoid & right ) const
	{
		return ( _semiMajorAxis == right._semiMajorAxis && _semiMinorAxis == right._semiMinorAxis
			&& _dPowFlattening == right._dPowFlattening); 
	}
};

//����54����ϵ��׼���� ��������˹������(1940 Krassovsky)  a=6378245m b=6356863.018773m ��=0.0033523298692 
//����80����ϵ��׼���� 1975��I.U.G.G�Ƽ�����(���ʴ�ز���Э��1975)��a=6378140m b=6356755.2881575m ��=0.0033528131778�������������������� 
//WGS-84 GPS ��׼����  WGS-84����(GPSȫ��λϵͳ����17����ʴ�ز���Э��) a=6378137m b=6356752.3142451m ��=0.00335281006247
struct CD_GeodesicDatum
{
	//std::string _strName;
	GCSType m_geo_crs_type;
	CD_PrimeMeridian _primeMeridian;
	CD_Ellipsoid _ellipsoid;

	CD_GeodesicDatum & operator = (const CD_GeodesicDatum &right)
	{
		//_strName = right._strName;
		m_geo_crs_type = right.m_geo_crs_type;
		_primeMeridian = right._primeMeridian;
		_ellipsoid = right._ellipsoid;

		return *this;
	}

	bool operator == ( const CD_GeodesicDatum & right ) const
	{
		return ( _primeMeridian == right._primeMeridian && _ellipsoid == right._ellipsoid); // type is not evaluated
	}
};
#pragma pack(pop)

}//namespace IS_GIS
#endif