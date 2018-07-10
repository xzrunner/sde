#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_LEAF_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_LEAF_H_

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
			class Leaf : public Node
			{
			public:
				virtual ~Leaf();

			private:
				Leaf(NVRTree* pTree, id_type id);

				virtual bool findLeafPath(id_type nodeID, const Rect& nodeMBR, 
					std::stack<std::pair<id_type, size_t> >* pathBuffer);

				friend class NVRTree;
				friend class BulkLoader;

				friend class NVDataPublish::LayerBulkLoader;

			}; // Leaf
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_LEAF_H_
