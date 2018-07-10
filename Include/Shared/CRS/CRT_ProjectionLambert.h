#ifndef IS_GIS_CRT_PROJECTIONLAMBERT_H_
#define IS_GIS_CRT_PROJECTIONLAMBERT_H_

#include"CRT_Projection.h"

namespace IS_GIS 
{
	
//�����ص����ͶӰ
class CRT_ProjectionLambert : public CRT_Projection
{
public:
	CRT_ProjectionLambert()
	{
//		this->m_projectType = PRJ_LAMBERT;
	};

	~CRT_ProjectionLambert()
	{
	};

	void SetOriginLatitude(double originLatitude);

	double GetOriginLatitude();

	void SetStandardParallel1(double standard_p1);

	double GetStandardParallel1();

	CRT_Projection* copy() const;

	IS_GIS::Pos3D projectFromGeographic( const IS_GIS::Pos3D& posSource ) const;

	IS_GIS::Pos3D inverseToGeographic( const IS_GIS::Pos3D& posSource ) const;

	virtual bool isInversable() const {	return true;	}
	virtual CRT* createInverseCRT() const;
	virtual bool operator == (const CRT &right) const;

	virtual PrjType prjType() const {	return PRJ_LAMBERT;	}
protected:
	double m_Standard_Parallel1;	//ͶӰ����γ��
	double m_OriginLatitude;		//ԭ��γ��

	double m_Central_Meridian;
};

//����������ͶӰ
class CRT_ProjectionLambertConformal : public CRT_Projection
{
public:
	CRT_ProjectionLambertConformal()
	{
//		this->m_projectType = PRJ_LAMBERT_CONFORMAL;
	};

	~CRT_ProjectionLambertConformal()
	{
	};

	void SetOriginLatitude(double originLatitude);

	double GetOriginLatitude();

	void SetStandardParallel1(double standard_p1);

	double GetStandardParallel1();

	void SetStandardParallel2(double standard_p2);

	double GetStandardParallel2();

	CRT_Projection* copy() const;

	IS_GIS::Pos3D projectFromGeographic( const IS_GIS::Pos3D& posSource) const;

	IS_GIS::Pos3D inverseToGeographic( const IS_GIS::Pos3D& posSource ) const;
	virtual bool isInversable() const {	return true;	}
	virtual CRT* createInverseCRT() const;
	virtual bool operator == (const CRT &right) const;

	virtual PrjType prjType() const {	return PRJ_LAMBERT_CONFORMAL;	}
protected:
	double m_Standard_Parallel1;//��������Ա�����ȱ�������ͶӰ�������ͶӰ
	double m_Standard_Parallel2;
	double m_OriginLatitude; //ԭ��γ��

	double m_Central_Meridian;//���뾭��	
};
	
}

#endif//IS_GIS_CRT_PROJECTIONLAMBERT_H_