#include "PointAgent.h"
#include "PropertyBuffer.h"
#include "../../BasicType/Point.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Agent;

PointAgent::PointAgent(IShape& s) 
	: m_point(dynamic_cast<Point*>(&s))
{
	if (!m_point)
		throw Tools::IllegalArgumentException("PointAgent::PointAgent: Type error.");

	m_propBuf = new PropertyBuffer(this);
}

PointAgent::~PointAgent()
{
	delete m_propBuf;
}

//
// ISerializable interface
//

size_t PointAgent::getByteArraySize() const
{
	return sizeof(Point*);
}

void PointAgent::loadFromByteArray(const byte* data)
{
	memcpy(m_point, data, sizeof(Point*));
}

void PointAgent::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, m_point, sizeof(Point*));
}

//
// IAgent interface
//

void PointAgent::getAgentProperty(const std::string& property, Tools::Variant& var) const
{
	m_propBuf->loadProperty(property, var);

	if (var.m_varType == Tools::VT_EMPTY)
	{
		if (property == "Coordinates")
		{
			std::vector<MapPos2D>* coords = new std::vector<MapPos2D>;
			coords->push_back(MapPos2D(m_point->m_pCoords[0], m_point->m_pCoords[1]));
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = coords;
		}
		else if (property == "GeoShape")
		{
			std::vector<IShape*>* shapes = new std::vector<IShape*>;
			shapes->push_back(m_point);
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = shapes;
		}
		else
			throw Tools::IllegalArgumentException("PointAgent::getAgentProperty: error property. ");

		m_propBuf->storeProperty(property, var);
	}
}

//
// Internal
//

void PointAgent::storeToShapes(std::vector<IShape*>& shapes) const
{
	shapes.push_back(m_point);
}

void PointAgent::loadFromShapes(const std::vector<IShape*>& shapes)
{
	assert(shapes.size() == 1);
	m_point = dynamic_cast<Point*>(shapes.front());
	if (!m_point)
		throw Tools::IllegalArgumentException("PointAgent::loadFromShapes: type error. ");
}