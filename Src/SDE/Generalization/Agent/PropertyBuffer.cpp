#include "PropertyBuffer.h"
#include "../../BasicType/Scope.h"

using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Agent;

PropertyBuffer::PropertyBuffer(IAgent* agent)
	: m_agent(agent)
{
}

PropertyBuffer::~PropertyBuffer()
{
	clearBuffer();
}

void PropertyBuffer::clearBuffer()
{
	std::map<std::string, Tools::Variant>::iterator itr = m_pool.begin();
	for ( ; itr != m_pool.end(); ++itr)
		if (itr->second.m_varType == Tools::VT_PVOID)
			deleteProperty(itr->first, itr->second);

	m_pool.clear();
}

void PropertyBuffer::loadProperty(const std::string& property, Tools::Variant& var) const
{
	std::map<std::string, Tools::Variant>::const_iterator itr 
		= m_pool.find(property);
	if (itr != m_pool.end())
		var = itr->second;
	else
		var.m_varType = Tools::VT_EMPTY;
}

void PropertyBuffer::storeProperty(const std::string& property, Tools::Variant& var)
{
	std::map<std::string, Tools::Variant>::iterator itr 
		= m_pool.find(property);
	if (itr != m_pool.end())
	{
		deleteProperty(itr->first, itr->second);
		m_pool.erase(itr);
	}

	m_pool.insert(std::make_pair(property, var));
}

void PropertyBuffer::deleteProperty(const std::string& property, Tools::Variant& var) const
{
	if (property == "GeoScope")
		delete static_cast<MapScope*>(var.m_val.pvVal);
	else if (property == "GeoShape")
		delete static_cast<std::vector<IShape*>*>(var.m_val.pvVal);
	else if (property == "Coordinates")
		delete static_cast<std::vector<MapPos2D>*>(var.m_val.pvVal);
	else
		throw Tools::IllegalArgumentException("PropertyBuffer::deleteProperty: error property. ");
}