#include "NumberOfCategories.h"
#include "../../Measure/All/CategoriesNumber.h"
#include "../../Algorithm/Points/KMeansClustering.h"
#include "../../Algorithm/Points/ISODATA.h"
#include "../../Algorithm/Points/SettlementSpacingRatio.h"
#include "../../Algorithm/Points/StructuralSimplification.h"
#include "../../Utility/GeneralizationMath.h"
#include "../../../BasicType/Scope.h"

using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Constraint::Points;

NumberOfCategories::NumberOfCategories(IAgent* agent, double scaleTimes)
	: AbstractConstraint(agent, new Measure::All::CategoriesNumber, 3, 2)
{
	setGoalValue(scaleTimes);
}

void NumberOfCategories::proposePlans(std::vector<IAlgorithm*>& plans) const
{
	size_t evalVal = evaluateConstraint();
	if (evalVal == 1)
	{
		Tools::Variant var;
		IAlgorithm* algo;

		m_agent->getAgentProperty("CategoriesNumber", var);
		assert(var.m_varType == Tools::VT_ULONG);
		size_t totNum = var.m_val.ulVal;

		m_agent->getAgentProperty("GeoScope", var);
		assert(var.m_varType == Tools::VT_PVOID);
		MapScope* scope = static_cast<MapScope*>(var.m_val.pvVal);
		double minEdge = std::min(scope->xLength(), scope->yLength());

		// K_MEANS_CLUSTERING
		algo = new Algorithm::KMeansClustering(m_goalValue, minEdge / totNum);
		plans.push_back(algo);

		// ISODATA
		algo = new Algorithm::ISODATA(m_goalValue, minEdge / totNum);
		plans.push_back(algo);

		// SETTLEMENT_SPACING_RATIO
		algo = new Algorithm::SettlementSpacingRatio(m_goalValue);
		plans.push_back(algo);

		// ALPHA_SHAPE

		// STRUCTURAL_SIMPLIFICATION
		algo = new Algorithm::StructuralSimplification(m_goalValue);
		plans.push_back(algo);
	}
}

size_t NumberOfCategories::getEvaluateValue() const
{
	//Tools::Variant varNum;
	//m_measure->getMeasureValue(m_agent, varNum);
	//size_t currNum = varNum.m_val.ulVal;

	//double evalVal = static_cast<double>(currNum) / m_goalValue;
	//if (evalVal >= 0.95)
	//	return 5;
	//else if (evalVal >= 0.75)
	//	return 4;
	//else if (evalVal >= 0.5)
	//	return 3;
	//else if (evalVal >= 0.25)
	//	return 2;
	//else
	//	return 1;

	return 0;
}

void NumberOfCategories::setGoalValue(double scaleTimes)
{
	Tools::Variant varNum;
//	m_measure->getMeasureValue(m_agent, varNum);
	size_t totNum = varNum.m_val.ulVal;

	m_goalValue = Utility::GeneralizationMath::calTargetMapSymbolNum(totNum, scaleTimes);

	m_goalValue = static_cast<size_t>(totNum * /*sqrt*/(scaleTimes));
}