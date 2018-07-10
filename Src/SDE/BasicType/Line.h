#ifndef _IS_SDE_LINE_H_
#define _IS_SDE_LINE_H_
#include "../Tools/SpatialDataBase.h"
#include "BasicGMType.h"

namespace IS_SDE
{
	class Rect;

	class Line : public Tools::IObject, public virtual IShape
	{
	public:
		Line();
		Line(const double* pCoords, size_t size);
		Line(const std::vector<MapPos2D>& line);
		Line(const Line& l);
		virtual ~Line();

		Line& operator=(const Line& l);
		bool operator==(const Line& l) const;

		//
		// IObject interface
		//
		virtual Line* clone();

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

		void initialize(size_t posNum, const double* pCoords);
		void getBothEndsPoint(MapPos2D* s, MapPos2D* e) const;
		void getPoint(size_t index, MapPos2D* point) const;
		void getPoint(std::vector<MapPos2D>* points) const;
		size_t posNum() const { return m_posNum; }
		void moveControlPoi(size_t pointIndex, const MapPos2D& newPos);
		double getLength() const;

	private:
		void initialize(const double* pCoords); 
		void initialize(const std::vector<MapPos2D>& line);
		void initialize(); 
		void checkRoom(size_t size);

	public:
		static const int DIMENSION = 2;
		size_t m_posNum;
		double* m_pCoords;

		friend class Rect;
		//Attr ;

	}; // Line

}

#endif // _IS_SDE_LINE_H_

