#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_NODE_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_NODE_H_
#include "../BaseTree/BaseTreeNode.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class PartitionMap;
		}
	}

	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class NVRTree;

			class Node;
			typedef Tools::PoolPointer<Node> NodePtr;

			class Node : public BaseTreeNode
			{
			public:
				//
				// Tools::ISerializable interface
				//
				virtual size_t getByteArraySize() const;
				virtual void loadFromByteArray(const byte* data);
				virtual void storeToByteArray(byte** data, size_t& len) const;

				//
				// SpatialIndex::INode interface
				//
				virtual bool isIndex() const;
				virtual bool isLeaf() const;

				virtual size_t getNodeType() const;

				void updateOneRoadToMuti(size_t index, const std::vector<std::pair<IShape*, bool> >& roads);
				void deleteRoads(std::vector<size_t>& indexes);

				const Rect& getNodeMBR() const { return m_nodeMBR; }

			private:
				Node();
				Node(NVRTree* pTree, id_type id, size_t level, size_t capacity);

				virtual Node& operator=(const Node&);

				virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
				virtual void deleteEntry(size_t index);
				virtual void clearAllEntry(bool invalidateRegion = true);

				virtual void condenseTree(std::stack<std::pair<id_type, size_t> >* pathBuffer);

				virtual bool findLeafPath(id_type nodeID, const Rect& nodeMBR, 
					std::stack<std::pair<id_type, size_t> >* pathBuffer);

				virtual RectPtr getRectPtr() const;

			private:
				static const int DIMENSION = 2;

			private:
				NVRTree* m_pNVRTree;

				friend class Tools::PointerPool<Node>;
				friend class NVRTree;
				friend class Index;
				friend class Leaf;
				friend class BulkLoader;

				friend class NVDataPublish::Arrangement::PartitionMap;

			}; // Node
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_NODE_H_
