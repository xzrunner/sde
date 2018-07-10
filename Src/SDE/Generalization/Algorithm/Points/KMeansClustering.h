#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_K_MEANS_CLUSTERING_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_K_MEANS_CLUSTERING_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Algorithm
		{
			class KMeansClustering : public IAlgorithm, public IClustering
			{
				typedef std::vector<std::vector<MapPos2D> > CLUSTER_POS;

			public:
				KMeansClustering(size_t k, double threshold)
					: m_k(k), m_threshold(threshold) {}

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return POITNS_AGGREGATION; }

				//
				// IOperator IAlgorithm
				//
				virtual void clustering(const std::vector<IShape*>& src, std::vector<std::vector<IShape*> >& dest);

				static void computerCenter(const CLUSTER_POS& clusterPos, std::vector<MapPos2D>& centers);
				static void clusterToPos(const std::vector<MapPos2D>& src, const std::vector<MapPos2D>& centers, 
					CLUSTER_POS& clusterPos);

			private:
				void clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest);
				void clustering(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest) const;
				bool needRecluster(const std::vector<MapPos2D>& centers, const CLUSTER_POS& clusterPos) const;

			private:
				size_t m_k;
				double m_threshold;

			}; // KMeansClustering
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_K_MEANS_CLUSTERING_H_