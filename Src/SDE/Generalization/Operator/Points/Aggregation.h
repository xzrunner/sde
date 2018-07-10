//#ifndef _IS_SDE_GENERALIZATION_OPERATOR_POINTS_AGGREGATION_H_
//#define _IS_SDE_GENERALIZATION_OPERATOR_POINTS_AGGREGATION_H_
//#include "../../GeneralizationBase.h"
//
//namespace IS_SDE
//{
//	namespace Generalization
//	{
//		namespace Operator
//		{
//			namespace Points
//			{
//				class Aggregation : public IOperator
//				{
//				public:
//					Aggregation(IAgent* agent, IAlgorithm* algo) : m_agent(agent), m_algo(algo) {}
//					virtual ~Aggregation() {
//						for_each(m_dest.begin(), m_dest.end(), Tools::DeletePointerFunctor<IShape>());
//					}
//
//					//
//					// IOperator interface
//					//
//					//virtual void implement();
//					//virtual const std::vector<IShape*>& getResult() const { return m_dest; }
//
//				private:
//					IAgent* m_agent;
//					IAlgorithm* m_algo;
//
//					std::vector<IShape*> m_dest;
//				};
//			}
//		}
//	}
//}
//
//#endif // _IS_SDE_GENERALIZATION_OPERATOR_POINTS_AGGREGATION_H_