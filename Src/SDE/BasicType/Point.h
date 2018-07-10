#ifndef _IS_SDE_POINT_H_
#define _IS_SDE_POINT_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	class Rect;
	class Line;

	class Point : public Tools::IObject, public virtual IShape
	{
	public:
		Point();
		Point(const MapPos2D& p);
		Point(const double* pCoords);
		Point(const Point& p);
		virtual ~Point();

		Point& operator=(const Point& p);
		bool operator==(const Point& p) const;

		//
		// IObject interface
		//
		virtual Point* clone();

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

		double getMinimumDistance(const Point& in) const;
		double getMinimumDistance(const Line& in) const;
		double getCoordinate(size_t index) const;

		void initialize(const double* pCoords);
		void initialize();

	private:
		bool isEmpty() const { return m_pCoords == NULL; }

	public:
		static const int DIMENSION = 2;
		double* m_pCoords;

		friend class Rect;

	}; // Point

	const size_t POINT_DATA_SIZE = sizeof(double) * Point::DIMENSION;
}

#endif // _IS_SDE_POINT_H_

