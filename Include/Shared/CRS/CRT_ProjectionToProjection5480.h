#ifndef IS_GIS_CRT_PROJECTION_TO_PROJECTION_5480_H_
#define IS_GIS_CRT_PROJECTION_TO_PROJECTION_5480_H_

#include "CRT_Projection.h"
#include "../Pack/Packable.h"
#include "datum.h"	

namespace IS_GIS {
	;

	class CRT_ProjectionToProjection5480 : public CRT_Projection, public Packable {
	public:
		CRT_ProjectionToProjection5480();
		CRT_ProjectionToProjection5480(const CRT_ProjectionToProjection5480& right);
		~CRT_ProjectionToProjection5480();


		virtual bool operator == (const CRT &right) const ;
		virtual CRT *copy() const ;
		virtual bool isInversable() const { return false; }
		virtual CRT *createInverseCRT() const { return NULL; } 
		virtual PrjType prjType() const { return PRJ_GAUSS; }

		virtual int_32 trans(const Pos3D &src, Pos3D *dest) const ;
		
		virtual int_32 calDXY(double x80,double y80,const IS_GIS::Pos3D& shift_val,const IS_GIS::Pos3D& posSource);

		virtual int calDXDY(const IS_GIS::Pos3D& pos3d54,const IS_GIS::Pos3D& pos3d80);

		virtual int_32 packSize(const void *how_to = NULL) const;
		virtual int_32 pack(byte_8 *buffer, int_32 buffer_size, byte_8 **end_pos, const void *how_to = NULL) const;
		virtual int_32 pack(byte_8 **result, const void *how_to = NULL)const;
		virtual int_32 unpack(const byte_8 *buffer, int_32 buf_size = PACK_IGNORE_BUF_SIZE);


	public:
		CD_Ellipsoid m_ellipsoid;
		double m_DX, m_DY; //�������ƫ����
		double m_x, m_y; // ����ƽ���������ϵ��ͼ�����˹ƽ������ֵ
		bool m_b54To80;
		bool m_b80To54;	

		double m_OriginLatitude;	//ԭ��γ��
		double m_Scale_Factor;		//�߶ȱ任����
		double m_false_east;
		double m_false_north;
		double m_central_meridian;	//����������
	};

}

#endif