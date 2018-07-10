#include "StateControl.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Agent;

//
// class OptList
//

OptList::OptList(const std::vector<IAlgorithm*>& opts)
	: m_optList(opts), m_nextOptIdx(0)
{
}

OptList::~OptList()
{
	for_each(m_optList.begin(), m_optList.end(), Tools::DeletePointerFunctor<IAlgorithm>());
}

//
// class AgentState
//

AgentState::AgentState(Tools::SmartPointer<AgentState> parent, double happiness)
	: m_parent(parent), m_happiness(happiness), m_data(NULL)
{
}

AgentState::~AgentState()
{
	delete m_data;
}

void AgentState::setOptList(IAgent* agent)
{
	std::vector<IAlgorithm*> opts;
	agent->proposePlans(opts);
	m_optList = Tools::SmartPointer<OptList>(new OptList(opts));
}

AgentState* AgentState::returnNextState(IAgent* agent) const
{
	if (m_optList->hasNextOpt())
	{
		IAlgorithm* opt = m_optList->getNextOpt();
		agent->triggerBestPlan(opt);
		double happiness = agent->calHappiness();
		return new AgentState(Tools::SmartPointer<AgentState>(const_cast<AgentState*>(this)), happiness);
	}

	if (m_parent.get() == NULL)
		return NULL;
	else
		// backtrack
		return m_parent->returnNextState(agent);
}

//
// class StateControl
//

StateControl::StateControl()
	: m_stateType(ST_NOT_ACTIVE), m_currState(NULL), m_bestState(NULL)
{
}

bool StateControl::setNextState(IAgent* agent)
{
	if (m_stateType == ST_PASSIVE)
		return false;
	else if (m_stateType == ST_NOT_ACTIVE)
	{
		AgentState* state = new AgentState(Tools::SmartPointer<AgentState>(), agent->calHappiness());
		m_stateType = ST_ACTIVE;
		m_currState = Tools::SmartPointer<AgentState>(state);
		m_bestState = m_currState;
	}
	else
	{
		if (m_currState->getHappiness() < m_bestState->getHappiness())
			m_currState = m_currState->getParent();
		else
		{
			m_currState->setOptList(agent);
			m_bestState = m_currState;
		}

		AgentState* nextState = m_currState->returnNextState(agent);
		if (!nextState)
			m_stateType = ST_PASSIVE;
		else
		{
			m_currState = Tools::SmartPointer<AgentState>(nextState);
			m_currState->storeAgent(agent);
		}
	}
	return true;
}
