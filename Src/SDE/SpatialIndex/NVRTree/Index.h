#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_INDEX_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_INDEX_H_

namespace IS_SDE
{
	namespace NVDataPublish
	{
		class LayerBulkLoader;
	}

	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class Index : public Node
			{
			public:
				virtual ~Index();

			private:
				Index(NVRTree* pTree, id_type id, size_t level);

				virtual bool findLeafPath(id_type nodeID, const Rect& nodeMBR, 
					std::stack<std::pair<id_type, size_t> >* pathBuffer);

				void adjustTree(Node* n, size_t index, std::stack<std::pair<id_type, size_t> >* pathBuffer);

				friend class NVRTree;
				friend class Node;
				friend class BulkLoader;

				friend class NVDataPublish::LayerBulkLoader;

			}; // Index
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_INDEX_H_

