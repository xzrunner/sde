#ifndef IS_GIS_CRT_H_
#define IS_GIS_CRT_H_

#include "extern.h"
//#include "crs.h"
//#include "CRS_Factory.h"
//#include "ComputeEquation.h"
//using namespace IS_GIS;
namespace IS_GIS 
{
;
struct Pos3D;

class CRT
{
	//DECLARE_INSTANCE_TRACE("CRT")
public:

	CRT(){}
	virtual ~CRT()	{}
	virtual bool operator == (const CRT &right) const = 0;
	virtual CRT *copy() const = 0;
	virtual bool isInversable() const = 0;
	virtual CRT *createInverseCRT() const = 0;

	// only Pos3D - sth. oop is unable to solve
	virtual int_32 trans(const Pos3D &src, Pos3D *dest) const = 0;
	//virtual CRT_Type *type() const = 0;
	int_32 batchTrans(const vector<Pos2D> &src, vector<Pos2D> *dest) const;
	int_32 batchTrans(const vector<Pos3D> &src, vector<Pos3D> *dest) const;
	int_32 trans(const Pos2D &src, Pos2D *dest) const;
};


} // namespace IS_GIS

#endif