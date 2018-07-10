#ifndef _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_NODE_H_
#define _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_NODE_H_
#include "../../../Tools/SpatialDataBase.h"
#include "../../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class LayerBulkLoader;
		}

		namespace Network
		{
			class PartitionMap;
			class AdjacencyList;
		}
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class StaticRTree;
				class Node;

				typedef Tools::PoolPointer<Node> NodePtr;

				class Node : public INode
				{
				public:
					virtual ~Node();

					//
					// Tools::IObject interface
					//
					virtual Tools::IObject* clone();

					//
					// Tools::ISerializable interface
					//
					virtual size_t getByteArraySize() const;
					virtual void loadFromByteArray(const byte* data);
					virtual void storeToByteArray(byte** data, size_t& len) const;

					//
					// SpatialIndex::IEntry interface
					//
					virtual id_type getIdentifier() const;
					virtual void getShape(IShape** out) const;

					//
					// SpatialIndex::INode interface
					//
					virtual size_t getChildrenCount() const;
					virtual id_type getChildIdentifier(size_t index)  const;
					virtual void setChildIdentifier(size_t index, id_type id);
					virtual void getChildShape(id_type id, IShape** out)  const;
					virtual void getChildShape(size_t index, IShape** out)  const;
					virtual size_t getLevel() const;
					virtual bool isIndex() const;
					virtual bool isLeaf() const;

					RectPtr getRectPtr() const;

					static size_t getIndexByteArraySize(size_t cChild);

				public:
					// Road ID struct:
					static const size_t EDGE_ID_SIZE			= 8;

					// Node info
					static const size_t CHILDREN_COUNT_SIZE		= 2;
					static const size_t MBR_SIZE				= 2 * 2 * 4;

					// Each child info
					static const size_t EACH_CHILD_SIZE_SIZE	= 3;

				private:
					size_t m_children;
						// The number of children pointed by this node.

					Rect m_nodeMBR;
						// The minimum bounding Rect enclosing all data contained in the node.

					byte** m_pData;
						// The data stored in the node.

					RectPtr* m_ptrMBR;
						// The corresponding data MBRs.

					id_type* m_pIdentifier;
						// The corresponding data identifiers.

					size_t* m_pDataLength;
						// The length of each child. 
						// No value is null when it is leaf node.

				private:
					Node(StaticRTree* pTree, id_type id);

					void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);

					static void readMBRFromByte(Rect& r, const byte*& ptr);
					static void writeMBRToByte(const Rect& r, byte*& ptr);

				private:
					id_type m_identifier;
						// The unique ID of this node.

					mutable bool m_bLeaf;
						// The Node is Index or Leaf.

					StaticRTree* m_tree;
						// Point to SpatialIndex

					size_t m_totalDataLength;

					friend class Tools::PointerPool<Node>;
					friend class StaticRTree;
					friend class PointBindLine;
					friend class NVDataPublish::Features::LayerBulkLoader;
					friend class NVDataPublish::Network::PartitionMap;
					friend class NVDataPublish::Network::AdjacencyList;

				}; // Node
			}
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_NODE_H_