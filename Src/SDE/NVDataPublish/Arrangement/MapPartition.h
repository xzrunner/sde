#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MAP_PARTITION_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MAP_PARTITION_H_
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class NVRTree;
		}
	}

	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class PartitionMap
			{
			public:
				PartitionMap(const std::vector<ISpatialIndex*>& src, size_t maxNum);
				~PartitionMap();

			private:
				void build();

				bool shouldSplitGrid(const Rect& scope) const;

			private:
				class Grid
				{
				public:
					Grid(const Rect& scope, bool bLeaf);
					~Grid();

					void split(PartitionMap& pm);

				public:
					Rect m_scope;
					bool m_bLeaf;
					Grid* m_children[4];

				}; // Grid

				class CountRoadVisitor : public ICountVisitor
				{
				public:
					CountRoadVisitor(size_t limit) : m_upperLimit(limit), m_count(0) {}

					virtual void countNode(const INode& in);
					virtual void countData(const INode& in, size_t iData, const IShape& query);
					virtual bool overUpperLimit() const;

				private:
					size_t m_upperLimit;
					size_t m_count;

				}; // CountRoadVisitor

			private:
				std::vector<IS_SDE::SpatialIndex::NVRTree::NVRTree*> m_layers;
				size_t m_maxNum;
				Grid* m_root;
				std::vector<Grid*> m_allLeaves;

				friend class Grid;
				friend class Arrange;

			}; // PartitionMap
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_MAP_PARTITION_H_
