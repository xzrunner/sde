/*	CoordConvertor.h
*	clm, qzy
*	2005-5-20
*/

#ifndef _COORDCONVERT_H_
#define _COORDCONVERT_H_

#include <vector>
#include "CRT_head.h"

namespace IS_GIS
{

//地理坐标和投影坐标间的换算
class CoordConvertor
{
public:
	CoordConvertor();
	CoordConvertor( const CRS * src_crs, const CRS * dest_crs = NULL, const CRT_Projection * crt = NULL,
		int_32 prj_type = 0);
	~CoordConvertor();

	void setPrjType(int_32 type);

public:
	const CRS * getSrcCrs() const { return m_crs; }
	int_32 setSrcCrs( const CRS * crs );

	const CRS * getDestCrs() const { return m_dest_crs; }
	int_32 setDestCrs( const CRS * crs );

	const CRT_Projection * getCrt() const { return m_crt; }
	int_32 setCrt( const CRT_Projection * crt );

	bool operator == ( const CoordConvertor & right ) const;
	CoordConvertor & operator = ( const CoordConvertor & right );

public:
	int_32 srcToDestPrj( double in_src_x, double in_src_y, double * out_prj_x, double * out_prj_y ) const;
	int_32 srcToDestPrj( const std::vector<Pos2D> & in_src_pos, std::vector<Pos2D> * out_prj_pos ) const;
	int_32 srcToDestPrj( const std::vector<Pos3D> & in_src_pos, std::vector<Pos3D> * out_prj_pos ) const;

	int_32 destPrjToSrc( double in_prj_x, double in_prj_y, double * out_src_x, double * out_src_y ) const;
	int_32 destPrjToSrc( const std::vector<Pos2D> & in_prj_pos, std::vector<Pos2D> * out_src_pos ) const;
	int_32 destPrjToSrc( const std::vector<Pos3D> & in_prj_pos, std::vector<Pos3D> * out_src_pos ) const;

	int_32 srcToDest( double in_src_x, double in_src_y, double* out_dest_x, double* out_dest_y ) const;
	int_32 srcToDest( const std::vector<Pos2D> & in_src_pos, std::vector<Pos2D> * out_dest_pos ) const;
	int_32 srcToDest( const std::vector<Pos3D> & in_src_pos, std::vector<Pos3D> * out_dest_pos ) const;

	int_32 destToSrc( double in_dest_x, double in_dest_y, double* out_src_x, double* out_src_y ) const;
    int_32 destToSrc( const std::vector<Pos2D> & in_dest_pos, std::vector<Pos2D> * out_src_pos ) const;
	int_32 destToSrc( const std::vector<Pos3D> & in_dest_pos, std::vector<Pos3D> * out_src_pos ) const;

protected:
	int_32 noConvert( double in_x, double in_y, double* out_x, double* out_y ) const;
	int_32 noConvert( const std::vector<Pos2D> & in_pos, std::vector<Pos2D> * out_pos ) const;
	int_32 noConvert( const std::vector<Pos3D> & in_pos, std::vector<Pos3D> * out_pos ) const;

	int_32 toGeo( const CRS* org_crs, const CRS* target_crs, const std::vector< Pos2D > & org_pos,
						std::vector< Pos2D > * ret_pos ) const;
	int_32 toGeo( const CRS* org_crs, const CRS* target_crs, const std::vector< Pos3D > & org_pos,
						std::vector< Pos3D > * ret_pos ) const;

	int_32 geoToGeo( const CRS* org_crs, const CRS* target_crs, const std::vector< Pos2D > & org_pos,
						std::vector< Pos2D > * ret_pos ) const;
	int_32 geoToGeo( const CRS* org_crs, const CRS* target_crs, const std::vector< Pos3D > & org_pos,
						std::vector< Pos3D > * ret_pos ) const;

	//下列方法中，如果crt与crt不对应则可能出现意外结果
	int_32 geoToPrj( const CRS* crs, const CRT_Projection* crt, const std::vector<Pos2D> & geo_pos, std::vector<Pos2D> * prj_pos ) const;
	int_32 geoToPrj( const CRS* crs, const CRT_Projection* crt, const std::vector<Pos3D> & geo_pos, std::vector<Pos3D> * prj_pos ) const;

	int_32 prjToGeo( const CRS* crs, const CRT_Projection* crt, const std::vector<Pos2D> & prj_pos, std::vector<Pos2D> * geo_pos ) const;
	int_32 prjToGeo( const CRS* crs, const CRT_Projection* crt, const std::vector<Pos3D> & prj_pos, std::vector<Pos3D> * geo_pos ) const;
private:
	CRS_RealWorld * m_crs;
	CRS_RealWorld * m_dest_crs;//目标crs
	CRT_Projection * m_crt;

	CRT_Projection * m_src_crt;//与源CRS绑定的CRT（防止反复因创建临时CRT降低效率）
	CRT_Projection * m_dest_crt;//与目标CRS绑定的CRT（防止反复创建临时CRT降低效率率）

	// patch
	int_32 m_prj_type; // 0: default, 1: 6 degree, 2: 3 degree

	CRT_Projection * newDefultCrt() const;
};

}

#endif//#define _COORDCONVERT_H_