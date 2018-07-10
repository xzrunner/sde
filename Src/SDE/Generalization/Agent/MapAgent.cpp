#include "MapAgent.h"
#include "GroupAgent.h"
#include "PointsAgent.h"
#include "PropertyBuffer.h"
#include "../Algorithm/Points/ISODATA.h"
#include "../../SpatialIndex/Utility/QueryStrategy.h"
#include "../../BasicType/Point.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Agent;

const size_t MapAgent::NUM_PER_GROUP = 50;
const double MAP_EDGE_DIVIDE_THRESHOLD = 100.0;

MapAgent::MapAgent(const std::vector<ISpatialIndex*>& layers)
{
	std::vector<ISpatialIndex*>* indexes = new std::vector<ISpatialIndex*>;
	indexes->assign(layers.begin(), layers.end());
	m_tools.addTool(TT_SPATIAL_INDEXES, indexes);

	clusterToGroups();
}

MapAgent::~MapAgent()
{
	delete m_propBuf;

	for_each(m_groups.begin(), m_groups.end(), Tools::DeletePointerFunctor<GroupAgent>());
}

//
// ISerializable interface
//

size_t MapAgent::getByteArraySize() const
{
	return sizeof(size_t) + sizeof(GroupAgent*) * m_groups.size();
}

void MapAgent::loadFromByteArray(const byte* data)
{
	m_groups.clear();

	size_t num = 0;
	memcpy(&num, data, sizeof(size_t));
	data += sizeof(size_t);
	m_groups.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		GroupAgent* ga = NULL;
		memcpy(ga, data, sizeof(GroupAgent*));
		data += sizeof(GroupAgent*);
		m_groups.push_back(ga);
	}
}

void MapAgent::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	size_t num = m_groups.size();
	memcpy(ptr, &num, sizeof(size_t));
	ptr += sizeof(size_t);
	for (size_t i = 0; i < m_groups.size(); ++i)
	{
		memcpy(ptr, m_groups[i], sizeof(GroupAgent*));
		ptr += sizeof(GroupAgent*);
	}
}

//
// IAgent interface
//

void MapAgent::getAgentProperty(const std::string& property, Tools::Variant& var) const
{
	m_propBuf->loadProperty(property, var);

	if (var.m_varType == Tools::VT_EMPTY)
	{
		if (property == "CategoriesNumber")
		{
			var.m_varType = Tools::VT_ULONG;
			var.m_val.ulVal = static_cast<uint32_t>(getCategoriesNumber());
		}
		else if (property == "GeoScope")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getGeoScope();
		}
		else if (property == "GeoShape")
		{
			var.m_varType = Tools::VT_PVOID;
			var.m_val.pvVal = getGeoShape();
		}
		else
			throw Tools::IllegalArgumentException("MapAgent::getAgentProperty: error property. ");

		m_propBuf->storeProperty(property, var);
	}
}

//
// Internal
//

size_t MapAgent::getCategoriesNumber() const
{
	size_t num = 0;
	for (size_t i = 0; i < m_groups.size(); ++i)
	{
		Tools::Variant var;
		m_groups[i]->getAgentProperty("CategoriesNumber", var);
		if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalStateException("MapAgent::getAgentProperty: Didn't find property.");
		num += var.m_val.ulVal;
	}
	return num;
}

MapScope* MapAgent::getGeoScope() const
{
	Rect r;
	r.makeInfinite();

	void* tool = m_tools.fetchTool(TT_SPATIAL_INDEXES);
	if (tool)
	{
		std::vector<ISpatialIndex*>* indexes = static_cast<std::vector<ISpatialIndex*>*>(tool);
		for (size_t i = 0; i < indexes->size(); ++i)
		{
			SpatialIndex::LayerRegionQueryStrategy qs;
			(*indexes)[i]->queryStrategy(qs);
			r.combineRect(qs.m_indexedSpace);
		}
	}
	else
	{
		for (size_t i = 0; i < m_groups.size(); ++i)
		{
			Tools::Variant var;
			m_groups[i]->getAgentProperty("GeoScope", var);
			if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("MapAgent::getGeoScope: Didn't find property.");
			MapScope* scope = static_cast<MapScope*>(var.m_val.pvVal);
			Rect tmp(scope->leftLow(), scope->rightTop());
			r.combineRect(tmp);
		}
	}

	return new MapScope(r);
}

std::vector<IShape*>* MapAgent::getGeoShape() const
{
	std::vector<IShape*>* shapes = new std::vector<IShape*>;
	for (size_t i = 0; i < m_groups.size(); ++i)
	{
		Tools::Variant var;
		m_groups[i]->getAgentProperty("GeoShape", var);
		if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("MapAgent::getGeoShape: Didn't find property.");
		std::vector<IShape*>* groupShapes = (std::vector<IShape*>*)var.m_val.pvVal;
		copy(groupShapes->begin(), groupShapes->end(), back_inserter(*shapes));
	}

	return shapes;
}

void MapAgent::clusterToGroups()
{
	std::vector<IShape*>* shapes = getGeoShape();

	if (!shapes->empty())
	{
		IShape* shape = shapes->front();

		Point* point = dynamic_cast<Point*>(shape);
		if (point)
		{
			clusterPoints(*shapes);
			delete shapes;
			return;
		}

		// todo Line...

		throw Tools::IllegalArgumentException("MapAgent::clusterToGroups: error Geo type. ");
	}
}

void MapAgent::clusterPoints(const std::vector<IShape*>& shapes)
{
	size_t k = getCategoriesNumber();
	MapScope* scope = getGeoScope();
	double threshold = std::max(scope->xLength(), scope->yLength()) / MAP_EDGE_DIVIDE_THRESHOLD;

	IClustering* cluster = new Algorithm::ISODATA(k / NUM_PER_GROUP, threshold);
	std::vector<std::vector<IShape*> > groupShapes;
	cluster->clustering(shapes, groupShapes);

	m_groups.reserve(groupShapes.size());
	for (size_t i = 0; i < groupShapes.size(); ++i)
		m_groups.push_back(new PointsAgent(groupShapes[i]));

	delete cluster;
	delete scope;
}