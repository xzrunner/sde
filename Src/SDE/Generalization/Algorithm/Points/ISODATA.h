#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_ISODATA_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_ISODATA_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Algorithm
		{
			// Iterative Self-Organizing Data Analysis Technique Algorithm
			class ISODATA : public IAlgorithm, public IClustering
			{
				struct Cluster;

			public:
				ISODATA(size_t k);
				ISODATA(double threshold);
				ISODATA(size_t k, double threshold);
				virtual ~ISODATA();

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return POITNS_AGGREGATION; }

				//
				// IOperator IAlgorithm
				//
				virtual void clustering(const std::vector<IShape*>& src, std::vector<std::vector<IShape*> >& dest);

			private:
				void clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest);

				void splitClusters(const std::vector<MapPos2D>& pos);
				void mergeClusters(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest) const;

				static void calStandardDeviation(const std::vector<MapPos2D>& src, MapPos2D* center, double* snx, double* sny);
				static void splitPos(const std::vector<MapPos2D>& src, const MapPos2D& c0, const MapPos2D& c1, 
					std::vector<MapPos2D>& dest0, std::vector<MapPos2D>& dest1);

				struct Cluster
				{
					std::vector<MapPos2D> m_pos;
					MapPos2D m_center;
					double m_snx, m_sny;

					Cluster() {}
					Cluster(const std::vector<MapPos2D>& src) : m_pos(src) {
						init();
					}

					void init() {
						ISODATA::calStandardDeviation(m_pos, &m_center, &m_snx, &m_sny);
					}
				};

			private:
				size_t m_k;
				double m_xThreshold, m_yThreshold;

				std::vector<Cluster*> m_clusters;

			}; // ISODATA
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_ISODATA_H_