#ifndef _IS_SDE_POLYGON_H_
#define _IS_SDE_POLYGON_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	class Rect;

	class Polygon : public Tools::IObject, public virtual IShape
	{
	public:
		Polygon();
		Polygon(size_t ringNum, const size_t* posNum, const double* pCoords);
		Polygon(const Polygon& l);
		virtual ~Polygon();

		Polygon& operator=(const Polygon& l);
		bool operator==(const Polygon& l) const;

		//
		// IObject interface
		//
		virtual Polygon* clone();

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

		////double getMinimumDistance(const Point& p) const;
		////double getCoordinate(size_t index) const;

		void initialize(const size_t ringNum, const size_t* posNum, const double* pCoords);

		size_t posNum() const;

	private:
		void initialize(const size_t* posNum, const double* pCoords); 
		size_t getTotPosNum(size_t ringNum, size_t* posNum) const;
		void checkRoom(size_t ringNum, size_t* posNum);
		
	public:
		static const int DIMENSION = 2;
		size_t m_ringNum;
		size_t* m_posNum;
		double* m_pCoords;

	}; // Polygon

}

#endif // _IS_SDE_POLYGON_H_

