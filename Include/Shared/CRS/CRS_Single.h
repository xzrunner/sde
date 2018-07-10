#ifndef IS_GIS_CRS_SINGLE_H_
#define IS_GIS_CRS_SINGLE_H_

#include "CRS.h"
#include "CoordSystem.h"

namespace IS_GIS
{
;

class CRS_Single : public CRS
{
public:
	//CRS_Single()
	//{
	//}

	virtual ~CRS_Single()
	{
	}

	//virtual void setCoordinateSystem( const CoordinateSystem & refCs) ;
	//virtual CoordinateSystem getCoordinateSystem() const ;

	virtual bool isCompound() const {	return false;	}
protected:
	//CoordinateSystem m_refCs;
};

}

#endif //