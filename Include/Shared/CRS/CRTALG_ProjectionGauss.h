#ifndef IS_GIS_CRTALG_ProjectionGauss_H_
#define IS_GIS_CRTALG_ProjectionGauss_H_

//#include"CRT_Projection.h"
#include "extern.h"
#include "Datum.h"

namespace IS_GIS 
{
;

struct Pos3D;

struct ProjectXY
{
	//double fLatitude,fLogitude;
	double _fOriginatedX,_fOriginatedY;
	double _fDeltaX,_fDeltaY;
};

class CRTALG_ProjectionGauss 
{
private:
	static ProjectXY _projectGauss[18][7];  //��˹-������ͶӰ�����

public:
	//CRTALG_ProjectionGauss( )
	//{
	//	this->m_projectType = PRJ_GAUSS;
	//}

	~CRTALG_ProjectionGauss( )
	{
	}

	void SetOriginLatitude( double originLat ); //��������������

	double GetOriginLatitude( ); //��������������

	void SetScaleFactor( double scale_factor); //���ó߶ȱ任����

	double GetScaleFactor();		//���س߶ȱ任����

	CRTALG_ProjectionGauss* copy() const;

	IS_GIS::Pos3D project6DuZone(IS_GIS::Pos3D& posSource );	//6�ȷִ�ͶӰ

	IS_GIS::Pos3D inverse6DuZoneToGeographic(IS_GIS::Pos3D& posSource );//����6�ȷִ���������

	IS_GIS::Pos3D project3DuZone(IS_GIS::Pos3D& posSource );	//3�ȷִ�ͶӰ

	IS_GIS::Pos3D inverse3DuZoneToGeographic(IS_GIS::Pos3D& posSource );//����3�ȷִ���������

	IS_GIS::Pos3D projectFromGeographic( const IS_GIS::Pos3D& posSource) const; //ͶӰ��ͬһ������

	IS_GIS::Pos3D inverseToGeographic( const IS_GIS::Pos3D& posSource) const;   //����ص�������


	// 54ͶӰ���굽80ͶӰ�����ת��,DX, DYΪ����ƫ����.
	int_32 projectFromProject(double DX, double DY,double X00, double Y00,double& X11,double& Y11,bool is_54_to_80 = true) const;

	// ������ƽ�������DX��DY, posSource(B54��L54��0)
	IS_GIS::Pos3D getDXYFromGeography(double x80, double y80, const IS_GIS::Pos3D& shift_val, const IS_GIS::Pos3D& posSource) const; // ���ؽ����DX��DY��0��

	//IS_GIS::Pos3D selZoneProject(int zone, IS_GIS::Pos3D& posSource); //������zone��ͶӰ

	//double countDistance(IS_GIS::Pos3D& posBegin, IS_GIS::Pos3D& posEnd); //������˹ͶӰ������ľ��� 
	//virtual bool operator == (const CRT_Projection &right)
	//{
	//	const CRTALG_ProjectionGauss *_right = dynamic_cast<const CRTALG_ProjectionGauss*>(&right);
	//	if(!_right)
	//		return false;
	//	return CRT_Projection::operator ==(right) && m_OriginLatitude == _right->m_OriginLatitude
	//		&& m_Scale_Factor == _right->m_Scale_Factor;
	//}

	//virtual bool isInversable() const {	return true;	}
	//virtual CRT* createInverseCRT() const;
	//virtual bool operator == (const CRT &right) const;

	//virtual PrjType prjType() const {	return PRJ_GAUSS;	}
private:
	 void getProjectXY( double dB , double dL , double& dX , double& dY ) const;	//����γ�Ⱥ;��ȣ����ض�Ӧ��ƽ������
	 void inverseFromPrjXY(double dX, double dY, double& dB, double& dL) const;	//����ͶӰx,yֵ����γ��
	 double getPrimeArcLength(double dB) const;	//����γ�ȼ��������߻���
	 double getLatitudeFromArcLen(double arcLength) const;		//���������߻�������γ��
		 
public:
	double m_OriginLatitude;	//ԭ��γ��
	double m_Scale_Factor;		//�߶ȱ任����
	double m_False_East;
	double m_False_North;
	double m_Central_Meridian;	//����������
	CD_Ellipsoid _ellipsoid;
};



}

#endif //IS_GIS_CRTALG_ProjectionGauss_H_