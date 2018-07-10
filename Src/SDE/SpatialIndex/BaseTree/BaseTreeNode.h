#ifndef _IS_SDE_SPATIALINDEX_BASE_TREE_NODE_H_
#define _IS_SDE_SPATIALINDEX_BASE_TREE_NODE_H_
#include "../Utility/Utility.h"
#include "../../BasicType/Rect.h"
#include "../../Tools/PoolPointer.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace QuadTree
		{
			class ChangeAllNodeID;
		};

		class BaseTreeNode : public INode
		{
		public:
			virtual ~BaseTreeNode();

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

			virtual size_t getNodeType() const;		
			virtual RectPtr getRectPtr() const;

		protected:
			BaseTreeNode();
			BaseTreeNode(id_type id, size_t level, size_t capacity);

			virtual void insertEntry(size_t dataLength, byte* pData, const Rect& mbr, id_type id);
			virtual void deleteEntry(size_t index);
			//virtual void updateEntry(size_t oldIndex, const std::vector<IShape*>& newEntries);
			virtual void clearAllEntry(bool invalidateRegion = true);

		private:
			static const int DIMENSION = 2;

		protected:
			size_t m_level;
				// The level of the node in the tree.

			id_type m_identifier;
				// The unique ID of this node.

			size_t m_children;
				// The number of children pointed by this node.

			size_t m_capacity;
				// Specifies the node capacity.

			Rect m_nodeMBR;
				// The minimum bounding Rect enclosing all data contained in the node.

			byte** m_pData;
				// The data stored in the node.

			RectPtr* m_ptrMBR;
				// The corresponding data MBRs.

			id_type* m_pIdentifier;
				// The corresponding data identifiers.

			size_t* m_pDataLength;

			size_t m_totalDataLength;

			friend class Tools::PointerPool<BaseTreeNode>;
			template<class TNode, class TStatistics> friend class BaseTree;
			friend class FetchDataVisitor;
			friend class QuadTree::ChangeAllNodeID;
		};	// BaseTreeNode
	}
}

#endif // _IS_SDE_SPATIALINDEX_BASE_TREE_NODE_H_