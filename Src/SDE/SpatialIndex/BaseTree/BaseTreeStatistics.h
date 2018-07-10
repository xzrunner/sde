#ifndef _IS_SDE_SPATIALINDEX_BASE_TREE_STATISTICS_H_
#define _IS_SDE_SPATIALINDEX_BASE_TREE_STATISTICS_H_
#include "../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		class BaseTreeStatistics : public IStatistics
		{
		public:
			BaseTreeStatistics();
			BaseTreeStatistics(const BaseTreeStatistics&);
			virtual ~BaseTreeStatistics();
			BaseTreeStatistics& operator=(const BaseTreeStatistics&);

			//
			// IStatistics interface
			//
			virtual size_t getReads() const;
			virtual size_t getWrites() const;
			virtual size_t getNumberOfNodes() const;
			virtual size_t getNumberOfData() const;

			virtual size_t getHits() const;
			virtual size_t getMisses() const;
			virtual size_t getQueryResults() const;
			virtual size_t getTreeHeight() const;
			virtual size_t getNumberOfNodesInLevel(size_t l) const;

		protected:
			void reset();

			size_t m_reads;

			size_t m_writes;

			size_t m_hits;

			size_t m_misses;

			size_t m_nodes;

			size_t m_queryResults;

			size_t m_data;

			size_t m_treeHeight;

			std::vector<size_t> m_nodesInLevel;

			template<class TNode, class TStatistics> friend class BaseTree;
		}; // BaseTreeStatistics
	}
}

#endif // _IS_SDE_SPATIALINDEX_BASE_TREE_STATISTICS_H_

