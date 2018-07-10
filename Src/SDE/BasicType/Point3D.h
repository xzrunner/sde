#ifndef _IS_SDE_POINT3D_H_
#define _IS_SDE_POINT3D_H_
#include "../Tools/SpatialDataBase.h"
#include "Point.h"

namespace IS_SDE
{
//	class Rect;

	class Point3D : public Point, public virtual IShape
	{
	public:
		Point3D();
		Point3D(const double* pCoords);
		Point3D(const Point3D& p);
		virtual ~Point3D();

		Point3D& operator=(const Point3D& p);
		bool operator==(const Point3D& p) const;

		//
		// IObject interface
		//
		virtual Point3D* clone();

		//
		// ISerializable interface
		//
		virtual size_t getByteArraySize() const;
		virtual void loadFromByteArray(const byte* data);
		virtual void storeToByteArray(byte** data, size_t& length) const;

		//
		// IShape interface
		//
		virtual bool intersectsShape(const IShape& in) const;
		virtual bool containsShape(const IShape& in) const;
		virtual bool touchesShape(const IShape& in) const;
		virtual void getCenter(Point& out) const;
		virtual void getMBR(Rect& out) const;
		virtual double getArea() const;
		virtual double getMinimumDistance(const IShape& in) const;
		virtual void transGeoToPrj(const ICrt& crt);

	public:
		static const int DIMENSION = 3;
		double* m_pHeight;

	}; // Point3D

	const size_t POINT3D_DATA_SIZE = sizeof(double) * Point3D::DIMENSION;

}

#endif // _IS_SDE_POINT3D_H_

