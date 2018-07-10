#include "CategoriesNumber.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Measure::All;

void CategoriesNumber::getMeasureValue(Generalization::IAgent* agent, Tools::PropertySet& ps)
{
	Tools::Variant var;
	agent->getAgentProperty("CategoriesNumber", var);
	if (var.m_varType != Tools::VT_ULONG) throw Tools::IllegalStateException("CategoriesNumber::getMeasureValue: Didn't find property.");
	ps.setProperty("CategoriesNumber", var);
}

