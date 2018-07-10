//#ifndef _IS_SDE_GENERALIZATION_AGENT_ALGO_FACTORY_H_
//#define _IS_SDE_GENERALIZATION_AGENT_ALGO_FACTORY_H_
//#include "../GeneralizationBase.h"
//#include "../../BasicType/Scope.h"
//
//namespace IS_SDE
//{
//	namespace Generalization
//	{
//		namespace Agent
//		{
//			enum AlgoName
//			{
//				// POITNS_AGGREGATION
//				AN_K_MEANS_CLUSTERING,
//				AN_ISODATA,
//				// POINTS_SELECTIVE
//				AN_SETTLEMENT_SPACING_RATIO,
//				// POITNS_REGIONIZATION
//				AN_ALPHA_SHAPE,
//				// POINTS_SIMPLIFICATION
//				AN_STRUCTURAL_SIMPLIFICATION
//
//			}; // AlgoType
//
//			class AlgoFactory
//			{
//			public:
//				AlgoFactory(double scale) : m_scale(scale) {}
//
//				IAlgorithm* createAlgo(AlgoName type, IAgent* agent);
//
//			private:
//				IAlgorithm* createKMeansClusteringAlgo(IAgent* agent);
//				IAlgorithm* createISODATAAlgo(IAgent* agent);
//				
//			private:
//				static size_t getCategoriesNumber(IAgent* agent);
//				static MapScope* getGeoScope(IAgent* agent);
//
//			private:
//				const double m_scale;
//					// The ratio of source map's scale denominator to target map's.
//
//			}; // AlgoFactory
//		}
//	}
//}
//
//#endif // _IS_SDE_GENERALIZATION_AGENT_ALGO_FACTORY_H_
