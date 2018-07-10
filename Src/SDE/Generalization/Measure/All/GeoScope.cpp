#include "GeoScope.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Measure::All;

void GeoScope::getMeasureValue(Generalization::IAgent* agent, Tools::PropertySet& ps)
{
	Tools::Variant var;
	agent->getAgentProperty("GeoScope", var);
	if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("GeoScope::getMeasureValue: Didn't find property.");
	ps.setProperty("GeoScope", var);
}

