#include "GeoToPrj.h"
#include "../../BasicType/BasicGMType.h"
#include "../../../../Include/Shared/CRS/CRS.h"
#include "../../../../Include/Shared/CRS/CRT_Factory.h"
#include "../../../../Include/Shared/CRS/CRS_TypeEnum.h"
#include "../../../../Include/Shared/CRS/CRS_Factory.h"
#include "../../../../Include/Shared/CRS/CRS_Geography.h"

using namespace IS_GIS;
using namespace IS_SDE::NVDataPublish::Features;

IS_GIS::CRT* GeoToPrj::m_crt = NULL;

GeoToPrj::GeoToPrj(size_t precision)
{
	m_precision = std::pow(10.0, static_cast<double>(precision));

	if (m_crt == NULL)
	{
		CRS * crs1 = CRS_Factory::createCRS_Geography(GCS_WGS_1984);
		IS_GIS::CRS_Geography *crs_geo = dynamic_cast<CRS_Geography*>(crs1);
		CRS * crs2 = CRS_Factory::createCRS_Project(*crs_geo, PRJ_MERCATOR);
		CRT_Factory::createCRT(*crs1, *crs2, &m_crt);
		delete crs1;
		delete crs2;
	}
}

void GeoToPrj::trans(const IS_SDE::MapPos3D& src, IS_SDE::MapPos3D* dest) const
{
	IS_GIS::Pos3D s(src.d0, src.d1, src.d2), d;
	m_crt->trans(s, &d);
	dest->d0 = d.d0 * m_precision;
	dest->d1 = d.d1 * m_precision;
	dest->d2 = d.d2 * m_precision;
}