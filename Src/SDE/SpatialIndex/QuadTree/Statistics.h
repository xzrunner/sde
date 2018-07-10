#ifndef _IS_SDE_SPATIALINDEX_QUAD_TREE_STATISTICS_H_
#define _IS_SDE_SPATIALINDEX_QUAD_TREE_STATISTICS_H_
#include "../BaseTree/BaseTreeStatistics.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class Statistics : public BaseTreeStatistics
			{
			public:
				Statistics();
				Statistics(const Statistics&);
				virtual ~Statistics();
				Statistics& operator=(const Statistics&);

				size_t getDuplicateTotal() const;
				size_t getDuplicateObj() const;
				size_t getSplit() const;
				size_t getTreeRebuild() const;

			private:
				void reset();

				void resetIOInfo();
				void resetTreeInfo();
				void resetSearchInfo();

				size_t m_duplicateTotal;

				size_t m_duplicateObj;

				size_t m_split;

				size_t m_treeRebuild;

				friend class QuadTree;
				friend class Node;
				friend class Index;
				friend class Leaf;
				friend class Overflow;
				friend class BulkLoader;

				friend std::ostream& operator<<(std::ostream& os, const Statistics& s);
			}; // Statistics

			std::ostream& operator<<(std::ostream& os, const Statistics& s);
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUAD_TREE_STATISTICS_H_

