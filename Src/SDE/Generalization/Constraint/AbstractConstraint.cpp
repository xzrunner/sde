#include "AbstractConstraint.h"

using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Constraint;

AbstractConstraint::AbstractConstraint(IAgent* agent, IMeasure* measure, size_t importance, size_t priority)
	: m_agent(agent), m_measure(measure), m_importance(importance), m_priority(priority), m_bDirty(true)
{
}

AbstractConstraint::~AbstractConstraint()
{
	delete m_measure;
}

size_t AbstractConstraint::evaluateConstraint() const
{
	if (m_bDirty)
	{
		m_consEvaluate = getEvaluateValue();
		m_bDirty = false;
	}

	return m_consEvaluate;
}