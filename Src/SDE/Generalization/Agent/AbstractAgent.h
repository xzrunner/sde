#ifndef _IS_SDE_GENERALIZATION_AGENT_ABSTRACT_AGENT_H_
#define _IS_SDE_GENERALIZATION_AGENT_ABSTRACT_AGENT_H_
#include "../GeneralizationBase.h"
#include "Toolbox.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Agent
		{
			class PropertyBuffer;
			class StateControl;

			class AbstractAgent : public IAgent
			{
			public:
				AbstractAgent();
				virtual ~AbstractAgent();

				virtual void initConstraints(const std::vector<IConstraint*>& cons);
				virtual void proposePlans(std::vector<IAlgorithm*>& plans) const;
				virtual void triggerBestPlan(IAlgorithm* plan);
				virtual double calHappiness() const;
				virtual bool doNextOperation();

			protected:
				virtual void storeToShapes(std::vector<IShape*>& shapes) const = 0;
				virtual void loadFromShapes(const std::vector<IShape*>& shapes) = 0;

			private:
				void clearBuffer();

			private:
				class ConstraintPriorityCmp
				{
				public:
					bool operator() (const IConstraint* lhs, const IConstraint* rhs) const {
						return lhs->getPriority() > rhs->getPriority();
					}
				}; // ConstraintPriorityCmp

			protected:
				Toolbox m_tools;

				std::vector<IConstraint*> m_constraints;

				PropertyBuffer* m_propBuf;

				StateControl* m_stateControl;

			}; // AbstractAgent
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_ABSTRACT_AGENT_H_