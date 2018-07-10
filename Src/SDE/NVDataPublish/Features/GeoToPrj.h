#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_GEO_TO_PRJ_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_GEO_TO_PRJ_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../../../Include/Shared/CRS/CRT.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class GeoToPrj : public ICrt
			{
			public:
				GeoToPrj(size_t precision);

				virtual void trans(const MapPos3D& src, MapPos3D* dest) const; 

			private:
				static IS_GIS::CRT* m_crt;
				size_t m_precision;

			}; // GeoToPrj
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_GEO_TO_PRJ_H_
