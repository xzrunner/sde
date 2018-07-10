#ifndef _IS_GIS_CRTALG_GEOGRAPHY_TO_GEOGRAPHY_H_
#define _IS_GIS_CRTALG_GEOGRAPHY_TO_GEOGRAPHY_H_

#include "DatumType.h"
#include "Datum.h"

namespace IS_GIS 
{
	class CRTALG_GeographyToGeography {


	public:
		void geographyFromGeography(double B54,double L54,double dx,double dy,double dz,double& B80,double& L80) const; // ����54�������꣨B54, L54)���õ�80��������(B80,L80),dx,dy,dzΪת������) ;

	public:
		CD_Ellipsoid _ellipsoid;
	};



}
#endif 