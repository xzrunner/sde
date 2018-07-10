#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_STATISTICS_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_STATISTICS_H_
#include "../BaseTree/BaseTreeStatistics.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class Statistics : public BaseTreeStatistics
			{
			public:
				Statistics();
				Statistics(const Statistics&);
				virtual ~Statistics();
				Statistics& operator=(const Statistics&);

			private:
				void reset();

				friend class NVRTree;
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

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_STATISTICS_H_

