#ifndef IS_GIS_CRS_TYPE_ENUM_H_
#define IS_GIS_CRS_TYPE_ENUM_H_

// no namespace
enum GeographicCRSType
{
	GCS_UNKNOWN = 0,
	GCS_WGS_1984 = 1,
	GCS_BEIJING_1954 = 2,
	GCS_XIAN_1980 = 3,
	GCS_CUSTOM = 4,
	GCS_DEFAULT = GCS_WGS_1984//GCS_BEIJING_1954
};

enum ProjectionType
{
	PRJ_GAUSS,
	PRJ_GAUSS_INVERSE,
	PRJ_GAUSS_ZONED,
	PRJ_MERCATOR,
	PRJ_MERCATOR_TRANVERSE,
	PRJ_LAMBERT,
	PRJ_LAMBERT_CONFORMAL,
	PRJ_UNKNOWN
};

enum EllipsoidType
{
	ETC_UNKNOWN,
	ETC_KRASSOVSKY_1940 = 1,
	ETC_CLARK_1856 = 2,
	ETC_CLARK_1863 = 3,
	ETC_CLARK_1866 = 4,
	ETC_CLARK_1880 = 5,
	ETC_BESSEL_1841 = 6,
	ETC_WGS_1984 = 7,
	ETC_EVEREST_1830 = 8,
	ETC_IAG_1975 = 9,
	ETC_DEFAULT = ETC_WGS_1984,
	ETC_LAST
};

#endif //IS_GIS_CRS_TYPE_ENUM_H_