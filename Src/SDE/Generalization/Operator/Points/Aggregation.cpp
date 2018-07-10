//#include "Aggregation.h"
//#include "../../Utility/AlgoFactory.h"
//
//using namespace IS_SDE;
//using namespace IS_SDE::Generalization::Operator::Points;
//
//void Aggregation::setAlgo(const Generalization::AlgoFactory* factory, 
//							   Generalization::AlgoName algo)
//{
//	m_algo = factory->createAlgo(algo);
//
//	if (m_algo->type() != POITNS_AGGREGATION)
//	{
//		delete m_algo;
//		throw Tools::IllegalStateException("Aggregation::setAlgorithm: Type error.");
//	}
//}
//
//void Aggregation::implement()
//{
//	std::vector<IShape*> src;
//	m_agent->getOriginalShapes(src);
//	m_algo->implement(src, m_dest);
//}