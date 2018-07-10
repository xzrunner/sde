#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_ALPHA_SHAPE_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_ALPHA_SHAPE_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Algorithm
		{
			class AlphaShape : public IAlgorithm
			{
			public:
				AlphaShape(size_t k);
				virtual ~AlphaShape();

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return POITNS_REGIONIZATION; }

			private:
				void clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest) const;

			private:
				IClustering* m_clusterAlgo;

			}; // AlphaShape
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_ALPHA_SHAPE_H_