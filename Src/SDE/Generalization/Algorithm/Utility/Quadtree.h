#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_QUADTREE_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_QUADTREE_H_
#include "../../GeneralizationBase.h"
#include "../../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Algorithm
		{
			class QTreeNode;

			class QTreeData
			{
			public:
				size_t m_id;
				IShape* m_data;

				QTreeData(size_t id, IShape* shape)
					: m_id(id), m_data(shape) {}

			}; // QTreeData

			class QTreeNode
			{
			public:
				QTreeNode(const Rect& scope) : m_scope(scope) { m_children[0] = NULL; }
				QTreeNode(const Rect& scope, const std::vector<QTreeData*>& datas, size_t capacity);
				~QTreeNode() {
					for_each(m_datas.begin(), m_datas.end(), Tools::DeletePointerFunctor<QTreeData>());
				}

				void queryByScope(const Rect& r, std::vector<QTreeData*>& datas);

			private:
				bool isLeaf() const { return m_children[0] == NULL; }

			private:
				Rect m_scope;

				QTreeNode* m_children[4];

				std::vector<QTreeData*> m_datas;

			}; // RTreeNode

			class QTree
			{
			public:
				QTree(const std::vector<IShape*>& shapes, size_t capacity);
				~QTree() { delete m_root; }

				void queryByScope(const Rect& r, std::vector<QTreeData*>& datas);

			private:
				void build(const std::vector<IShape*>& shapes, size_t capacity);

			private:
				QTreeNode* m_root;
				
			}; // RTree
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_QUADTREE_H_