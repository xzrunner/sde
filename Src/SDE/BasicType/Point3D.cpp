#include "Point3D.h"
#include "Rect.h"
using namespace IS_SDE;

Point3D::Point3D() : Point(), m_pHeight(NULL)
{
}

Point3D::Point3D(const double* pCoords) : Point(pCoords)
{
	m_pHeight = new double(pCoords[2]);
}

Point3D::Point3D(const Point3D& p) : Point(p.m_pCoords)
{
	m_pHeight = new double(*p.m_pHeight);
}

Point3D::~Point3D()
{
}

Point3D& Point3D::operator=(const Point3D& p)
{
	if (this != &p)
	{
		memcpy(m_pCoords, p.m_pCoords, Point::DIMENSION * sizeof(double));
		*m_pHeight = *p.m_pHeight;
	}
	return *this;
}

bool Point3D::operator==(const Point3D& p) const
{
	if (p.Point::operator == (*this))
		return *m_pHeight == *p.m_pHeight;
	else
		return false;
}

//
// IObject interface
//
Point3D* Point3D::clone()
{
	return new Point3D(*this);
}

//
// ISerializable interface
//
size_t Point3D::getByteArraySize() const
{
	return DIMENSION * sizeof(double);
}

void Point3D::loadFromByteArray(const byte* data)
{
	if (NULL == m_pCoords)
	{
		try
		{
			m_pCoords = new double[Point::DIMENSION];
		}
		catch (...)
		{
			delete[] m_pCoords;
			throw;
		}
	}

	if (NULL == m_pHeight)
	{
		try
		{
			m_pHeight = new double;
		}
		catch (...)
		{
			delete m_pHeight;
			throw;
		}
	}

	memcpy(m_pCoords, data, Point::getByteArraySize());
	data += Point::getByteArraySize();
	memcpy(m_pHeight, data, sizeof(double));
}

void Point3D::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, m_pCoords, Point::getByteArraySize());
	ptr += Point::getByteArraySize();
	memcpy(ptr, m_pHeight, sizeof(double));
}

//
// IShape interface
//

bool Point3D::intersectsShape(const IShape& s) const
{
	return Point::intersectsShape(s);
}

bool Point3D::containsShape(const IShape& s) const
{
	return Point::containsShape(s);
}

bool Point3D::touchesShape(const IShape& s) const
{
	return Point::touchesShape(s);
}

void Point3D::getCenter(Point& p) const
{
	Point::getCenter(p);
}

void Point3D::getMBR(Rect& out) const
{
	Point::getMBR(out);
}

double Point3D::getArea() const
{
	return Point::getArea();
}

double Point3D::getMinimumDistance(const IShape& s) const
{
	return Point::getMinimumDistance(s);
}

void Point3D::transGeoToPrj(const ICrt& crt)
{
	MapPos3D src(m_pCoords[0], m_pCoords[1], m_pCoords[2]), dest;
	crt.trans(src, &dest);
	m_pCoords[0] = dest.d0;
	m_pCoords[1] = dest.d1;
	m_pCoords[2] = dest.d2;
}