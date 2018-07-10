#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_DOUGLAS_PEUCKER_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_DOUGLAS_PEUCKER_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	class Line;

	namespace Generalization
	{
		namespace Algorithm
		{
			class DouglasPeucker : public IAlgorithm
			{
			public:
				DouglasPeucker(double precision) : m_precision(precision) {}

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return LINE_POINT_REDUCTION; }

			private:
				void implement(const Line* src, Line* dest);
				void initMiddleData(const Line* l);
				void pointReduction(size_t begin, size_t end);
				void getResult(Line* l) const;

			private:
				const double m_precision;
				std::vector<std::pair<MapPos2D, bool> > m_tmpPos;

			}; // DouglasPeucker
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_DOUGLAS_PEUCKER_H_