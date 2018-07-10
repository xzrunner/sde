//#include "AlgoFactory.h"
//#include "../Utility/GeneralizationMath.h"
//#include "../Algorithm/Points/KMeansClustering.h"
//#include "../Measure/GeoScope.h"
//#include "../Measure/CategoriesNumber.h"
//
//using namespace IS_SDE;
//using namespace IS_SDE::Generalization;
//using namespace IS_SDE::Generalization::Agent;
//
//IAlgorithm* AlgoFactory::createAlgo(AlgoName type, IAgent* agent)
//{
//	switch (type)
//	{
//	case AN_K_MEANS_CLUSTERING:
//		return createKMeansClusteringAlgo(agent);
//	case AN_ISODATA:
//		return createISODATAAlgo(agent);
//	default:
//		Tools::IllegalArgumentException("AlgoFactory::createAlgo: AlgoName error.");
//		return NULL;
//	}
//}
//
//IAlgorithm* AlgoFactory::createKMeansClusteringAlgo(IAgent* agent)
//{
//	size_t sNum = getCategoriesNumber(agent);
//	size_t tNum = Utility::GeneralizationMath::calTargetMapSymbolNum(sNum, m_scale);
//
//	MapScope* s = getGeoScope(agent);
//	double minEdge = std::min(s->xLength(), s->yLength());
//	delete s;
//
//	return new Algorithm::KMeansClustering(tNum, minEdge / sNum);
//}
//
//IAlgorithm* AlgoFactory::createISODATAAlgo(IAgent* agent)
//{
//	size_t sNum = getCategoriesNumber(agent);
//	size_t tNum = Utility::GeneralizationMath::calTargetMapSymbolNum(sNum, m_scale);
//
//	MapScope* s = getGeoScope(agent);
//	double minEdge = std::min(s->xLength(), s->yLength());
//	delete s;
//
//	return new Algorithm::ISODATA();
//}
//
//size_t AlgoFactory::getCategoriesNumber(IAgent* agent)
//{
//	Measure::CategoriesNumber measure;
//	Tools::Variant var;
//	measure.getMeasureValue(agent, var);
//	return var.m_val.lVal;
//}
//
//MapScope* AlgoFactory::getGeoScope(IAgent* agent)
//{
//	Measure::GeoScope measure;
//	Tools::Variant var;
//	measure.getMeasureValue(agent, var);
//	return (MapScope*)var.m_val.pvVal;
//}