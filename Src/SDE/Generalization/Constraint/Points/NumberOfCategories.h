#ifndef _IS_SDE_GENERALIZATION_CONSTRAINT_POINTS_NUMBER_OF_CATEGORIES_H_
#define _IS_SDE_GENERALIZATION_CONSTRAINT_POINTS_NUMBER_OF_CATEGORIES_H_
#include "../AbstractConstraint.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Constraint
		{
			namespace Points
			{
				class NumberOfCategories : public AbstractConstraint
				{
				public:
					NumberOfCategories(IAgent* agent, double scaleTimes);

					virtual void proposePlans(std::vector<IAlgorithm*>& plans) const;

				protected:
					virtual size_t getEvaluateValue() const;

				private:
					void setGoalValue(double scaleTimes);

				}; // NumberOfCategories
			}
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_CONSTRAINT_POINTS_NUMBER_OF_CATEGORIES_H_
