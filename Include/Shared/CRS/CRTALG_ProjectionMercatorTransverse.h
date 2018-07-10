#ifndef IS_GIS_CRTALG_PROJECTIONMERCATOR_H_
#define IS_GIS_CRTALG_PROJECTIONMERCATOR_H_

#include "extern.h"
#include "Datum.h"

namespace IS_GIS 
{
;
	
class CRTALG_ProjectionMercatorTransverse
{
public:
	~CRTALG_ProjectionMercatorTransverse()
	{
	};

	void SetCentralMeridian(double centralMeridian);

	double GetCentralMeridian();

	void SetOriginLatitude( double originLat ); //��������������

	double GetOriginLatitude( ); //��������������

	void SetScaleFactor( double scale_factor); //���ó߶ȱ任����

	double GetScaleFactor();		//���س߶ȱ任����


	CRTALG_ProjectionMercatorTransverse* copy() const;

	IS_GIS::Pos3D projectFromGeographic( const IS_GIS::Pos3D& posSource) const;

	IS_GIS::Pos3D inverseToGeographic( const IS_GIS::Pos3D& posSource ) const;
public:
	double m_OriginLatitude;	//ԭ��γ��
	double m_Scale_Factor;		//�߶ȱ任����
	double m_False_East;
	double m_False_North;
	CD_Ellipsoid _ellipsoid;
	double m_Central_Meridian;	//����������
};


}//namespace IS_GIS

#endif //IS_GIS_CRT_PROJECTIONMERCATOR_H_