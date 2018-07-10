#include "Line.h"
#include "Rect.h"
#include "../Tools/SpatialMath.h"
using namespace IS_SDE;

Line::Line() 
	: m_pCoords(NULL), m_posNum(0)
{
}

Line::Line(const double* pCoords, size_t size)
	: m_posNum(size)
{
	initialize(pCoords);
}

Line::Line(const std::vector<MapPos2D>& line)
	: m_posNum(line.size())
{
	assert(m_posNum > 1);
	initialize(line);
}

Line::Line(const Line& l)
	: m_posNum(l.m_posNum)
{
	initialize(l.m_pCoords);
}

Line::~Line()
{
	delete[] m_pCoords;
}

Line& Line::operator=(const Line& l)
{
	if (this != &l)
	{
		checkRoom(l.m_posNum);
		m_posNum = l.m_posNum;
		memcpy(m_pCoords, l.m_pCoords, DIMENSION * sizeof(double) * m_posNum);
	}
	return *this;
}

bool Line::operator==(const Line& l) const
{
	if (l.m_posNum != m_posNum)
		return false;
	for (size_t i = 0; i < m_posNum; ++i)
	{
		if (
			m_pCoords[i * 2] < l.m_pCoords[i * 2] - Tools::DOUBLE_TOLERANCE ||
			m_pCoords[i * 2] > l.m_pCoords[i * 2] + Tools::DOUBLE_TOLERANCE ||
			m_pCoords[i * 2 + 1] < l.m_pCoords[i * 2 + 1] - Tools::DOUBLE_TOLERANCE ||
			m_pCoords[i * 2 + 1] > l.m_pCoords[i * 2 + 1] + Tools::DOUBLE_TOLERANCE
			)
			return false;
	}
	return true;
}

//
// IObject interface
//
Line* Line::clone()
{
	return new Line(*this);
}

//
// ISerializable interface
//

size_t Line::getByteArraySize() const
{
	return
		sizeof(byte) +							// type
		sizeof(size_t) +						// pos num
		sizeof(double) * DIMENSION * m_posNum;
}

void Line::loadFromByteArray(const byte* data)
{
	data += sizeof(byte);	// type
	memcpy(&m_posNum, data, sizeof(size_t));
	data += sizeof(size_t);
	initialize();
	memcpy(m_pCoords, data, sizeof(double) * DIMENSION * m_posNum);
}

void Line::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	ShapeType type = ST_LINE;
	memcpy(ptr, &type, sizeof(byte));
	ptr += sizeof(byte);
	memcpy(ptr, &m_posNum, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, m_pCoords, sizeof(double) * DIMENSION * m_posNum);
	// ptr += sizeof(double) * DIMENSION * m_posNum;

}

//
// IShape interface
//

bool Line::intersectsShape(const IShape& s) const
{
	const Rect* pr = dynamic_cast<const Rect*>(&s);
	if (pr != 0) 
		return pr->intersectsLine(*this);

	//const Point* ppt = dynamic_cast<const Point*>(&s);
	//if (ppt != 0) 
	//	return intersectsPoint(*ppt);

	//const Line* pl = dynamic_cast<const Line*>(&s);
	//if (pl != 0)
	//	return intersectsLine(*pl);

	throw Tools::IllegalStateException(
		"Line::intersectsShape: Not implemented yet!"
		);
}

bool Line::containsShape(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

bool Line::touchesShape(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Line::getCenter(Point& out) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Line::getMBR(Rect& out) const
{
	out.makeInfinite();

	for (size_t i = 0; i < m_posNum; ++i)
	{
		size_t iX = i * 2, iY = i * 2 + 1;

		if (m_pCoords[iX] > out.m_pHigh[0])
			out.m_pHigh[0] = m_pCoords[iX];
		if (m_pCoords[iX] < out.m_pLow[0])
			out.m_pLow[0] = m_pCoords[iX];

		if (m_pCoords[iY] > out.m_pHigh[1])
			out.m_pHigh[1] = m_pCoords[iY];
		if (m_pCoords[iY] < out.m_pLow[1])
			out.m_pLow[1] = m_pCoords[iY];
	}
}

double Line::getArea() const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

double Line::getMinimumDistance(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Line::transGeoToPrj(const ICrt& crt)
{
	MapPos3D dest;
	for (size_t i = 0; i < m_posNum; ++i)
	{
		size_t iX = i * 2, iY = i * 2 + 1;
		MapPos3D src(m_pCoords[iX], m_pCoords[iY], 0.0);
		crt.trans(src, &dest);
		m_pCoords[iX] = dest.d0;
		m_pCoords[iY] = dest.d1;
	}
}

void Line::initialize(size_t posNum, const double* pCoords)
{
	if (m_posNum != 0)
		throw Tools::IllegalStateException("Line::initialize: m_posNum != 0, only use to empty Line. ");
	m_posNum = posNum;
	initialize(pCoords);
}

void Line::getBothEndsPoint(MapPos2D* s, MapPos2D* e) const
{
	getPoint(0, s);
	getPoint(m_posNum - 1, e);
}

void Line::getPoint(size_t index, MapPos2D* point) const
{
	if (index >= m_posNum)
		throw Tools::IndexOutOfBoundsException(index);
	point->d0 = m_pCoords[index * 2];
	point->d1 = m_pCoords[index * 2 + 1];
}

void Line::getPoint(std::vector<MapPos2D>* points) const
{
	points->reserve(m_posNum);
	for (size_t i = 0; i < m_posNum; ++i)
		points->push_back(MapPos2D(m_pCoords[i * 2], m_pCoords[i * 2 + 1]));
}

void Line::moveControlPoi(size_t pointIndex, const MapPos2D& newPos)
{
	if (pointIndex >= m_posNum)
		throw Tools::IndexOutOfBoundsException(pointIndex);
	m_pCoords[pointIndex * 2] = newPos.d0;
	m_pCoords[pointIndex * 2 + 1] = newPos.d1;
}

double Line::getLength() const
{
	double ret = 0.0;
	for (size_t i = 0; i < m_posNum - 1; ++i)
	{
		ret += Tools::SpatialMath::getDistance(
			m_pCoords[i * 2], m_pCoords[i * 2 + 1], m_pCoords[i * 2 + 2], m_pCoords[i * 2 + 3]
			);
	}
	return ret;
}

void Line::initialize(const double* pCoords)
{
	initialize();
	memcpy(m_pCoords, pCoords, DIMENSION * sizeof(double) * m_posNum);
}

void Line::initialize(const std::vector<MapPos2D>& line)
{
	initialize();
	memcpy(m_pCoords, &line.front(), DIMENSION * sizeof(double) * m_posNum);
}

void Line::initialize()
{
	try
	{
		m_pCoords = new double[DIMENSION * m_posNum];
	}
	catch (...)
	{
		delete[] m_pCoords;
		throw;
	}
}

void Line::checkRoom(size_t size)
{
	if (m_posNum != size)
	{
		delete[] m_pCoords;
		m_pCoords = new double[DIMENSION * size];
	}
}
