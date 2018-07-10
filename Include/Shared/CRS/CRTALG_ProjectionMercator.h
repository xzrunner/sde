#ifndef IS_GIS_CRTALG_PROJECTIONMERCATOR_H_
#define IS_GIS_CRTALG_PROJECTIONMERCATOR_H_

#include "extern.h"
#include "Datum.h"

namespace IS_GIS 
{
;
	
class CRTALG_ProjectionMercator
{
public:
	~CRTALG_ProjectionMercator()
	{
	};

	void SetCentralMeridian(double centralMeridian);

	double GetCentralMeridian();

	void SetStandardParallel1(double standard_p1);

	double GetStandardParallel1();

	CRTALG_ProjectionMercator* copy() const;

	IS_GIS::Pos3D projectFromGeographic( const IS_GIS::Pos3D& posSource) const;

	IS_GIS::Pos3D inverseToGeographic( const IS_GIS::Pos3D& posSource ) const;
public:
	double m_Standard_Parallel1;//伸缩变形控制 = l*cos(m_Standard_Parallel1)
	double m_False_East;
	double m_False_North;
	double m_Central_Meridian;	//中央子午线
	CD_Ellipsoid _ellipsoid;
};


}//namespace IS_GIS

#endif //IS_GIS_CRT_PROJECTIONMERCATOR_H_