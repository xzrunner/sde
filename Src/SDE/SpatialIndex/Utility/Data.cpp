#include "Data.h"
#include "../../BasicType/Point.h"
#include "../../BasicType/Point3D.h"
#include "../../BasicType/Line.h"
#include "../../BasicType/Polygon.h"
using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex;

Data::Data(size_t len, byte* pData, Rect& r, id_type id)
	: m_id(id), m_rect(r), m_pData(NULL), m_dataLength(len)
{
	if (m_dataLength > 0)
	{
		m_pData = new byte[m_dataLength];
		memcpy(m_pData, pData, m_dataLength);
	}
}

Data::Data(const IShape* s, id_type id)
	: m_id(id), m_pData(NULL), m_dataLength(0)
{
	s->storeToByteArray(&m_pData, m_dataLength);
	s->getMBR(m_rect);
}

Data::~Data()
{
	delete[] m_pData;
}

Data* Data::clone()
{
	return new Data(m_dataLength, m_pData, m_rect, m_id);
}

id_type Data::getIdentifier() const
{
	return m_id;
}

void Data::getShape(IShape** out) const
{
	if (m_dataLength == 0)
		*out = new Rect(m_rect);
	else
		getCertainShape(out);
}

void Data::getData(size_t& len, byte** data) const
{
	len = m_dataLength;
	*data = 0;

	if (m_dataLength > 0)
	{
		*data = new byte[m_dataLength];
		memcpy(*data, m_pData, m_dataLength);
	}
}

size_t Data::getByteArraySize() const
{
	return
		sizeof(id_type) +
		sizeof(size_t) +
		m_dataLength +
		m_rect.getByteArraySize();
}

void Data::loadFromByteArray(const byte* ptr)
{
	memcpy(&m_id, ptr, sizeof(id_type));
	ptr += sizeof(id_type);

	delete[] m_pData;
	m_pData = 0;

	memcpy(&m_dataLength, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	if (m_dataLength > 0)
	{
		m_pData = new byte[m_dataLength];
		memcpy(m_pData, ptr, m_dataLength);
		ptr += m_dataLength;
	}

	m_rect.loadFromByteArray(ptr);
}

void Data::storeToByteArray(byte** data, size_t& len) const
{
	len = getByteArraySize();

	*data = new byte[len];
	byte* ptr = *data;

	memcpy(ptr, &m_id, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, &m_dataLength, sizeof(size_t));
	ptr += sizeof(size_t);

	if (m_dataLength > 0)
	{
		memcpy(ptr, m_pData, m_dataLength);
		ptr += m_dataLength;
	}

	size_t rectsize;
	byte* rectdata = 0;
	m_rect.storeToByteArray(&rectdata, rectsize);

	memcpy(ptr, rectdata, rectsize);
	delete[] rectdata;
	// ptr += rectsize;
}

void Data::getCertainShape(IShape** s) const
{
	if (m_dataLength == POINT_DATA_SIZE)
	{
		*s = new Point;
		(*s)->loadFromByteArray(m_pData);
		return;
	}

	if (m_dataLength == POINT3D_DATA_SIZE)
	{
		*s = new Point3D;
		(*s)->loadFromByteArray(m_pData);
		return;
	}

	ShapeType shapeType = ST_POINT;
	byte* ptr = m_pData;
	memcpy(&shapeType, ptr, sizeof(byte));
	ptr += sizeof(byte);

	if (shapeType == ST_LINE)
	{
		*s = new Line;
		(*s)->loadFromByteArray(m_pData);
		return;
	}
	else if (shapeType == ST_POLYGON)
	{
		*s = new Polygon;
		(*s)->loadFromByteArray(m_pData);
		return;
	}

	throw Tools::IllegalStateException("Data::getShape: error ShapeType. ");
}