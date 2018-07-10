#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_MAP_PARTITION_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_MAP_PARTITION_H_
#include "../Base.h"
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
		namespace Network
		{
			class Node;
			class Publish;

			class PartitionMap
			{
			public:
				PartitionMap(const std::vector<STATIC_LAYER*>& src, size_t maxNum);
				~PartitionMap();

				void queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const;

				// page = 0, offset = 0, means empty leaf!
				void storeToStorageManager(Publish& pb, size_t pageSize, size_t& nextPage) const;
				void loadFromStorageManager(IStorageManager* sm, size_t firstIndexPage);

				static bool isPointInScope(const MapPos2D& p, const Rect& s);

			private:
				void build();

				bool shouldSplitNode(const Rect& scope) const;

			private:
				class CountNodeVisitor : public ICountVisitor
				{
				public:
					CountNodeVisitor(size_t limit) : m_upperLimit(limit), m_count(0) {}

					virtual void countNode(const INode& in);
					virtual void countData(const INode& in, size_t iData, const IShape& query);
					virtual bool overUpperLimit() const;

				private:
					size_t m_upperLimit;
					size_t m_count;

				}; // CountNodeVisitor

			private:
				const std::vector<STATIC_LAYER*>& m_layers;
				size_t m_maxNum;
				Node* m_root;
				std::vector<Node*> m_allLeaves;

				friend class Index;
				friend class Publish;

			}; // PartitionMap
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_MAP_PARTITION_H_
