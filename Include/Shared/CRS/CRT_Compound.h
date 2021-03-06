#ifndef IS_GIS_CRT_COMPOUND_H_
#define IS_GIS_CRT_COMPOUND_H_

#include "extern.h"
#include "CRT.h"

namespace IS_GIS
{
;

class CRT_Compound : public CRT
{
public:
	CRT_Compound();
	CRT_Compound(const CRT_Compound &right);
	virtual ~CRT_Compound();
	virtual bool operator == (const CRT &right) const;
	virtual CRT *copy() const;
	virtual bool isInversable() const;
	virtual CRT *createInverseCRT() const;

	// only Pos3D - sth. oop is unable to solve
	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const;
public:
	vector<CRT*> m_crt_list;
	typedef vector<CRT*>::iterator IterType;
	typedef vector<CRT*>::const_iterator ConstIterType;
};

}// IS_GIS

#endif //IS_GIS_CRT_COMPOUND_H_