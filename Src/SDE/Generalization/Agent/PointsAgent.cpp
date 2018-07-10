#include "PointsAgent.h"
#include "PointAgent.h"
#include "PropertyBuffer.h"
#include "../Measure/Points/MinimalDistance.h"
#include "../Measure/Points/SortedVoronoiArea.h"
#include "../Algorithm/Utility/Utility.h"
#include "../../BasicType/Scope.h"
#include "../../Tools/SpatialMath.h"
#include "../../Algorithms/ComputationalGeometry/DelaunayTriangulation.h"
#include "../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"

using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Agent;

PointsAgent::PointsAgent(const std::vector<IShape*>& src)
{
	for (size_t i = 0; i < src.size(); ++i)
		m_points.push_back(new PointAgent(*src[i]));

	m_propBuf = new PropertyBuffer(this);

	Tools::Variant var;
	getAgentProperty("Coordinates", var);
	if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::PointsAgent: Didn't find property.");
	std::vector<MapPos2D>* coords 
		= static_cast<std::vector<MapPos2D>*>(var.m_val.pvVal);

	addDelaunayTriangulationTool(*coords);
	addVoronoiTool(*coords);
}

PointsAgent::~PointsAgent()
{
	delete m_propBuf;
	for_each(m_points.begin(), m_points.end(), Tools::DeletePointerFunctor<PointAgent>());
}

//
// ISerializable interface
//

size_t PointsAgent::getByteArraySize() const
{
	return sizeof(size_t) + sizeof(PointAgent*) * m_points.size();
}

void PointsAgent::loadFromByteArray(const byte* data)
{
	m_points.clear();

	size_t num = 0;
	memcpy(&num, data, sizeof(size_t));
	data += sizeof(size_t);
	m_points.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		PointAgent* pa = NULL;
		memcpy(pa, data, sizeof(PointAgent*));
		data += sizeof(PointAgent*);
		m_points.push_back(pa);
	}
}

void PointsAgent::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	size_t num = m_points.size();
	memcpy(ptr, &num, sizeof(size_t));
	ptr += sizeof(size_t);
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		memcpy(ptr, m_points[i], sizeof(PointAgent*));
		ptr += sizeof(PointAgent*);
	}
}

//
// IAgent interface
//

void PointsAgent::getAgentProperty(const std::string& property, Tools::Variant& var) const
{
	m_propBuf->loadProperty(property, var);

	if (var.m_varType == Tools::VT_EMPTY)
	{
		if (property == "CategoriesNumber")
		{
			var.m_varType = Tools::VT_ULONG;
			var.m_val.ulVal = static_cast<uint32_t>(m_points.size());
			m_propBuf->storeProperty(property, var);
		}
		else if (property == "Coordinates")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getCoordinates();
			m_propBuf->storeProperty(property, var);
		}
		else if (property == "GeoScope")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getGeoScope();
			m_propBuf->storeProperty(property, var);
		}
		else if (property == "MinimalDistance")
		{
			MapPos2D* nearestPos = new MapPos2D[2];

			var.m_varType = Tools::VT_DOUBLE;
			var.m_val.dblVal = getMinimalDistance(nearestPos);
			m_propBuf->storeProperty(property, var);

			Tools::Variant coordsVar;
			coordsVar.m_varType = Tools::VT_PVOID;
			coordsVar.m_val.pvVal = new MapPos2D[2];
			m_propBuf->storeProperty("NearestTwoPoints", coordsVar);
		}
		else if (property == "NearestTwoPoints")
		{
			m_propBuf->loadProperty(property, var);
		}
		else if (property == "GeoShape")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getGeoShape();
			m_propBuf->storeProperty(property, var);
		}
		else if (property == "SortedVoronoiArea")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getSortedVoronoiArea();
			m_propBuf->storeProperty(property, var);
		}
		else
			throw Tools::IllegalArgumentException("PointsAgent::getAgentProperty: error property. ");
	}
}

//
// Internal
//

void PointsAgent::storeToShapes(std::vector<IShape*>& shapes) const
{
	Tools::Variant var;
	getAgentProperty("GeoShape", var);
	if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::storeToShapes: Didn't find property.");
	shapes = *static_cast<std::vector<IShape*>*>(var.m_val.pvVal);
}

void PointsAgent::loadFromShapes(const std::vector<IShape*>& shapes)
{
	m_points.clear();
	for (size_t i = 0; i < shapes.size(); ++i)
		m_points.push_back(new PointAgent(*shapes[i]));
}

void PointsAgent::addDelaunayTriangulationTool(const std::vector<MapPos2D>& src)
{
	Algorithms::DelaunayTriangulation* dt
		= new Algorithms::DelaunayTriangulation(src);
	m_tools.addTool(TT_DELAUNAY_TRIANGULATION, dt);
}

void PointsAgent::addVoronoiTool(const std::vector<MapPos2D>& src)
{
	Algorithms::VoronoiByDelaunay* voronoi 
		= new Algorithms::VoronoiByDelaunay(src);
	m_tools.addTool(TT_VORONOI, voronoi);
}

std::vector<MapPos2D>* PointsAgent::getCoordinates() const
{
	std::vector<MapPos2D>* coords = new std::vector<MapPos2D>;
	coords->reserve(m_points.size());
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		Tools::Variant var;
		m_points[i]->getAgentProperty("Coordinates", var);
		if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::getCoordinate: Didn't find property.");
		std::vector<MapPos2D>* pos 
			= static_cast<std::vector<MapPos2D>*>(var.m_val.pvVal);
		assert(pos->size() == 1);
		coords->push_back(pos->front());
	}

	return coords;
}

MapScope* PointsAgent::getGeoScope() const
{
	Tools::Variant varPos;
	getAgentProperty("Coordinates", varPos);
	if (varPos.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::getGeoScope: Didn't find property.");
	std::vector<MapPos2D>* points 
		= static_cast<std::vector<MapPos2D>*>(varPos.m_val.pvVal);

	MapScope* scope = new MapScope;
	Tools::SpatialMath::getMBR(*points, *scope);

	return scope;
}

double PointsAgent::getMinimalDistance(MapPos2D* nearestPos) const
{
	void* tool = m_tools.fetchTool(TT_DELAUNAY_TRIANGULATION);
	if (tool)
	{
		Algorithms::DelaunayTriangulation* dt 
			= static_cast<Algorithms::DelaunayTriangulation*>(tool);
		return Measure::Points::MinimalDistance::calMinimumDistance(dt, nearestPos[0], nearestPos[1]);
	}
	else
	{
		Tools::Variant varPos;
		getAgentProperty("Coordinates", varPos);
		if (varPos.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::getMinimalDistance: Didn't find property.");
		std::vector<MapPos2D>* coords 
			= static_cast<std::vector<MapPos2D>*>(varPos.m_val.pvVal);
		return Measure::Points::MinimalDistance::calMinimumDistance(*coords, nearestPos[0], nearestPos[1]);
	}
}

std::vector<IShape*>* PointsAgent::getGeoShape() const
{
	std::vector<IShape*>* shapes = new std::vector<IShape*>;
	shapes->reserve(m_points.size());
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		Tools::Variant var;
		m_points[i]->getAgentProperty("GeoShape", var);
		if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("PointsAgent::getGeoShape: Didn't find property.");
		std::vector<IShape*>* shape 
			= static_cast<std::vector<IShape*>*>(var.m_val.pvVal);
		assert(shape->size() == 1);
		shapes->push_back(shape->front());
	}

	return shapes;
}

byte* PointsAgent::getSortedVoronoiArea() const
{
	void* tool = m_tools.fetchTool(TT_VORONOI);
	assert(tool);
	Algorithms::VoronoiByDelaunay* voronoi 
		= static_cast<Algorithms::VoronoiByDelaunay*>(tool);

	std::vector<double> area;
	Measure::Points::SortedVoronoiArea::calSortedVoronoiArea(voronoi, area);
	size_t count = area.size();
	size_t len = sizeof(size_t) + sizeof(double) * count;
	byte* data = new byte[len];
	byte* ptr = data;
	memcpy(ptr, &count, sizeof(size_t));
	ptr += sizeof(size_t);
	for (size_t i = 0; i < area.size(); ++i)
	{
		memcpy(ptr, &area[i], sizeof(double));
		ptr += sizeof(double);
	}

	return data;
}