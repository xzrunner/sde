#include "Polygon.h"
#include "Rect.h"
using namespace IS_SDE;

Polygon::Polygon() 
	: m_ringNum(0), m_posNum(NULL), m_pCoords(NULL)
{
}

Polygon::Polygon(size_t ringNum, const size_t* posNum, const double* pCoords)
	: m_ringNum(ringNum)
{
	initialize(posNum, pCoords);
}

Polygon::Polygon(const Polygon& l)
	: m_ringNum(l.m_ringNum)
{
	initialize(l.m_posNum, l.m_pCoords);
}

Polygon::~Polygon()
{
	delete[] m_posNum;
	delete[] m_pCoords;
}

Polygon& Polygon::operator=(const Polygon& l)
{
	if (this != &l)
	{
		checkRoom(l.m_ringNum, l.m_posNum);
		m_ringNum = l.m_ringNum;
		memcpy(m_posNum, l.m_posNum, m_ringNum);
		memcpy(m_pCoords, l.m_pCoords, DIMENSION * sizeof(double) * m_ringNum);
	}
	return *this;
}

bool Polygon::operator==(const Polygon& l) const
{
	if (l.m_ringNum != m_ringNum)
		return false;

	size_t cursor = 0;
	for (size_t i = 0; i < m_ringNum; ++i)
	{
		if (m_posNum[i] != l.m_posNum[i])
			return false;
		for (size_t j = 0; j < l.m_posNum[i]; ++j)
		{
			if (
				m_pCoords[cursor * 2] < l.m_pCoords[cursor * 2] - Tools::DOUBLE_TOLERANCE ||
				m_pCoords[cursor * 2] > l.m_pCoords[cursor * 2] + Tools::DOUBLE_TOLERANCE ||
				m_pCoords[cursor * 2 + 1] < l.m_pCoords[cursor * 2 + 1] - Tools::DOUBLE_TOLERANCE ||
				m_pCoords[cursor * 2 + 1] > l.m_pCoords[cursor * 2 + 1] + Tools::DOUBLE_TOLERANCE
				)
				return false;
			++cursor;
		}
	}
	return true;
}

//
// IObject interface
//
Polygon* Polygon::clone()
{
	return new Polygon(*this);
}

//
// ISerializable interface
//
size_t Polygon::getByteArraySize() const
{
	return sizeof(size_t) 
		 + sizeof(size_t) * m_ringNum
		 + sizeof(double) * DIMENSION * getTotPosNum(m_ringNum, m_posNum);
}

void Polygon::loadFromByteArray(const byte* data)
{
	memcpy(&m_ringNum, data, sizeof(size_t));
	data += sizeof(size_t);
	size_t tmpSize = sizeof(size_t) * m_ringNum;
	memcpy(m_posNum, data, tmpSize);
	data += tmpSize;
	tmpSize = sizeof(double) * DIMENSION * getTotPosNum(m_ringNum, m_posNum);
	memcpy(m_pCoords, data, tmpSize);
	// data += tmpSize;
}

void Polygon::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, &m_ringNum, sizeof(size_t));
	ptr += sizeof(size_t);
	size_t tmpSize = sizeof(size_t) * m_ringNum;
	memcpy(ptr, m_posNum, tmpSize);
	ptr += tmpSize;
	tmpSize = sizeof(double) * DIMENSION * getTotPosNum(m_ringNum, m_posNum);
	memcpy(ptr, m_pCoords, tmpSize);
	// ptr += tmpSize;
}

//
// IShape interface
//
bool Polygon::intersectsShape(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

bool Polygon::containsShape(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

bool Polygon::touchesShape(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Polygon::getCenter(Point& out) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Polygon::getMBR(Rect& out) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

double Polygon::getArea() const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

double Polygon::getMinimumDistance(const IShape& in) const
{
	throw Tools::NotSupportedException("Have no time to finish it now. ");
}

void Polygon::transGeoToPrj(const ICrt& crt)
{
	MapPos3D dest;
	size_t cursor = 0;
	for (size_t i = 0; i < m_ringNum; ++i)
	{
		for (size_t j = 0; j < m_posNum[i]; ++j)
		{
			size_t iX = cursor * 2, iY = cursor * 2 + 1;
			MapPos3D src(m_pCoords[iX], m_pCoords[iY], 0.0);
			crt.trans(src, &dest);
			m_pCoords[iX] = dest.d0;
			m_pCoords[iY] = dest.d1;
			++cursor;
		}
	}
}

void Polygon::initialize(const size_t ringNum, const size_t* posNum, const double* pCoords)
{
	m_ringNum = ringNum;
	initialize(posNum, pCoords);
}

size_t Polygon::posNum() const
{
	return getTotPosNum(m_ringNum, m_posNum);
}

void Polygon::initialize(const size_t* posNum, const double* pCoords)
{
	try
	{
		m_posNum = new size_t[m_ringNum];
	}
	catch (...)
	{
		delete[] m_posNum;
		throw;
	}
	memcpy(m_posNum, posNum, m_ringNum);

	size_t totPosNum = getTotPosNum(m_ringNum, m_posNum);
	try
	{
		m_pCoords = new double[DIMENSION * totPosNum];
	}
	catch (...)
	{
		delete[] m_pCoords;
		throw;
	}

	memcpy(m_pCoords, pCoords, DIMENSION * sizeof(double) * totPosNum);
}

size_t Polygon::getTotPosNum(size_t ringNum, size_t* posNum) const
{
	size_t totPosNum = 0;
	for (size_t i = 0; i < ringNum; ++i)
		totPosNum += posNum[i];
	return totPosNum;
}

void Polygon::checkRoom(size_t ringNum, size_t* posNum)
{
	if (ringNum != m_ringNum)
	{
		delete[] m_posNum;
		m_posNum = new size_t[ringNum];
	}

	size_t newSize = getTotPosNum(ringNum, posNum);
	if (newSize != getTotPosNum(m_ringNum, m_posNum))
	{
		delete[] m_pCoords;
		m_pCoords = new double[DIMENSION * newSize];
	}
}