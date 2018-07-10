#include "AbstractAgent.h"
#include "PropertyBuffer.h"
#include "StateControl.h"

using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Agent;

AbstractAgent::AbstractAgent()
{
	m_propBuf = new PropertyBuffer(this);
	m_stateControl = new StateControl();
}

AbstractAgent::~AbstractAgent()
{
	delete m_stateControl;
	delete m_propBuf;
}

//
// IAgent interface
//

void AbstractAgent::initConstraints(const std::vector<IConstraint*>& cons)
{
	m_constraints.assign(cons.begin(), cons.end());
	sort(m_constraints.begin(), m_constraints.end(), ConstraintPriorityCmp());
}

void AbstractAgent::proposePlans(std::vector<IAlgorithm*>& plans) const
{
	for (size_t i = 0; i < m_constraints.size(); ++i)
		m_constraints[i]->proposePlans(plans);
}

void AbstractAgent::triggerBestPlan(IAlgorithm* plan)
{
	std::vector<IShape*> src, dest;
	storeToShapes(src);
	plan->implement(src, dest);
	loadFromShapes(dest);

	clearBuffer();
}

double AbstractAgent::calHappiness() const
{
	double totImpt = 0;
	for (size_t i = 0; i < m_constraints.size(); ++i)
		totImpt += m_constraints[i]->getImportance();

	double happiness = 0.0;
	for (size_t i = 0; i < m_constraints.size(); ++i)
		happiness += m_constraints[i]->evaluateConstraint() * m_constraints[i]->getImportance() / totImpt;

	return happiness;
}

bool AbstractAgent::doNextOperation()
{
	return m_stateControl->setNextState(this); 
}

//
// Internal
//

void AbstractAgent::clearBuffer()
{
	m_propBuf->clearBuffer();
	for (size_t i = 0; i < m_constraints.size(); ++i)
		m_constraints[i]->clearBuffer();
}