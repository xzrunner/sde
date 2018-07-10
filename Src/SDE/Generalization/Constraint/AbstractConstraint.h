#ifndef _IS_SDE_GENERALIZATION_CONSTRAINT_ABSTRACT_CONSTRAINT_H_
#define _IS_SDE_GENERALIZATION_CONSTRAINT_ABSTRACT_CONSTRAINT_H_
#include "../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Constraint
		{
			class AbstractConstraint : public IConstraint
			{
			public:
				AbstractConstraint(IAgent* agent, IMeasure* measure, size_t importance,
					size_t priority);
				virtual ~AbstractConstraint();

				virtual size_t evaluateConstraint() const;
				virtual size_t getImportance() const { return m_importance; }
				virtual size_t getPriority() const { return m_priority; }
				virtual void clearBuffer() { m_bDirty = true; }

			protected:
				virtual size_t getEvaluateValue() const = 0;

			protected:
				IAgent* m_agent;

				IMeasure* m_measure;

				size_t m_goalValue;

				size_t m_importance;

				size_t m_priority;

				mutable bool m_bDirty;
				mutable size_t m_consEvaluate;

			}; // AbstractConstraint
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_CONSTRAINT_ABSTRACT_CONSTRAINT_H_
