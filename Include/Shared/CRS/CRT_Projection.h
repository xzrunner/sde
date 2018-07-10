#ifndef IS_GIS_CRT_PROJECTION_H_
#define IS_GIS_CRT_PROJECTION_H_

#include "CRT.h"
#include "DatumType.h"

namespace IS_GIS
{
;

class CRT_Projection : public CRT
{
public:
	CRT_Projection(){}
	virtual ~CRT_Projection(){}

	//virtual CRT* copy() const = 0;

	virtual PrjType prjType() const  = 0;
};

}

#endif //IS_GIS_CRT_PROJECTION_H_