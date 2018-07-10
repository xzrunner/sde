#ifndef _IS_SDE_GENERALIZATION_AGENT_STATE_CONTROL_H_
#define _IS_SDE_GENERALIZATION_AGENT_STATE_CONTROL_H_
#include "../GeneralizationBase.h"
#include "../../Tools/SmartPointer.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Agent
		{
			class AgentState;

			class OptList
			{
			public:
				OptList(const std::vector<IAlgorithm*>& opts);
				~OptList();

				bool hasNextOpt() const { return m_nextOptIdx != m_optList.size(); }
				IAlgorithm* getNextOpt() {
					return hasNextOpt() ? m_optList[m_nextOptIdx++] : NULL;
				}

			private:
				std::vector<IAlgorithm*> m_optList;
				size_t m_nextOptIdx;

			}; // OptList

			class AgentState
			{
			public:
				AgentState(Tools::SmartPointer<AgentState> parent, double happiness);
				~AgentState();

				double getHappiness() const { return m_happiness; }
				Tools::SmartPointer<AgentState> getParent() const { return m_parent; }
				void setOptList(IAgent* agent);

				void loadAgent(IAgent* agent) const {
					agent->loadFromByteArray(m_data);
				}
				void storeAgent(IAgent* agent) {
					agent->storeToByteArray(&m_data, m_len);
				}

				AgentState* returnNextState(IAgent* agent) const;

			private:
				Tools::SmartPointer<OptList> m_optList;

				Tools::SmartPointer<AgentState> m_parent;
				double m_happiness;

				byte* m_data;
				size_t m_len;

			}; // AgentState

			class StateControl
			{
			public:
				StateControl();

				bool setNextState(IAgent* agent);

			private:
				enum StateType
				{
					ST_NOT_ACTIVE,
					ST_ACTIVE,
					ST_PASSIVE
				};

			private:
				StateType m_stateType;

				Tools::SmartPointer<AgentState> m_currState, m_bestState;
 
			}; // StateControl
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_STATE_CONTROL_H_