#ifndef _IS_SDE_SPATIALINDEX_RTREE_STATISTICS_H_
#define _IS_SDE_SPATIALINDEX_RTREE_STATISTICS_H_
#include "../BaseTree/BaseTreeStatistics.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace RTree
		{
			class Statistics : public BaseTreeStatistics
			{
			public:
				Statistics();
				Statistics(const Statistics&);
				virtual ~Statistics();
				Statistics& operator=(const Statistics&);

				virtual size_t getSplits() const;
				virtual size_t getAdjustments() const;

			private:
				void reset();

				size_t m_splits;

				size_t m_adjustments;

				friend class RTree;
				friend class Node;
				friend class Index;
				friend class Leaf;
				friend class BulkLoader;

				friend std::ostream& operator<<(std::ostream& os, const Statistics& s);
			}; // Statistics

			std::ostream& operator<<(std::ostream& os, const Statistics& s);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_RTREE_STATISTICS_H_

