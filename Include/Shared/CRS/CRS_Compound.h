#ifndef IS_GIS_CRS_COMPOUND_H_
#define IS_GIS_CRS_COMPOUND_H_

#include "CRS.h"

namespace IS_GIS
{
;
class CRT;

class CRS_Compound : public CRS
{
public:
	virtual ~CRS_Compound(){}
	virtual CRS *getRefCRS() const = 0;
	virtual CRT *getRefCRT() const = 0;
protected:
//	CRS *m_ref_crs;
//	CRS *m_ref_crt;
};

} // namespace

#endif //IS_GIS_CRS_COMPOUND_H_