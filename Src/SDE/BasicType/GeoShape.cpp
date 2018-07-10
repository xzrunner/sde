#include "GeoShape.h"
#include "Line.h"
using namespace IS_SDE;

//
// GeoShape
//

GeoShape::GeoShape(ISpatialIndex* index, size_t layerID, id_type nodeID, id_type objID, size_t objIndex)
	: m_layerID(layerID), m_nodeID(nodeID), m_objID(objID), m_objIndex(objIndex), m_index(index)
{
}

void GeoShape::getStorePosition(std::pair<id_type, size_t>* pos) const
{
	pos->first	= m_nodeID;
	pos->second = m_objIndex;
}

ISpatialIndex* GeoShape::getIndex() const
{
	return m_index;
}

//
// GeoLine
//

GeoLine::GeoLine(ISpatialIndex* index, Line* line, size_t layerID, id_type nodeID, id_type objID, size_t objIndex)
	: GeoShape(index, layerID, nodeID, objID, objIndex), m_line(line)
{
}

GeoLine::~GeoLine()
{
	delete m_line;
}

void GeoLine::getCoordinates(std::vector<MapPos2D>* coords) const
{
	return m_line->getPoint(coords);
}

size_t GeoLine::posSize() const
{
	return m_line->posNum();
}

IShape* GeoLine::getShape() const
{
	return m_line;
}

void GeoLine::moveCtlPoi(size_t index, const MapPos2D& newPos)
{
	m_line->moveControlPoi(index, newPos);
}