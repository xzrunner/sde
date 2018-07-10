#include "Quadtree.h"
#include "../../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

//
// class QTreeNode
//

QTreeNode::QTreeNode(const Rect& scope, const std::vector<QTreeData*>& datas, size_t capacity)
	: m_scope(scope)
{
	m_children[0] = 0;

	if (datas.size() <= capacity)
		m_datas.assign(datas.begin(), datas.end());
	else
	{
		Rect childScope(scope);
		for (size_t i = 0; i < 4; ++i)
		{
			Tools::SpatialMath::getRectChildGrid(m_scope, i, childScope);
			m_children[i] = new QTreeNode(childScope);
		}

		for (size_t i = 0; i < datas.size(); ++i)
			for (size_t j = 0; j < 4; ++j)
				if (m_children[j]->m_scope.intersectsShape(*datas[i]->m_data))
					m_children[j]->m_datas.push_back(datas[i]);
	}
}

void QTreeNode::queryByScope(const Rect& r, std::vector<QTreeData*>& datas)
{
	if (r.intersectsRect(m_scope))
	{
		if (!isLeaf())
			for (size_t i = 0; i < 4; ++i)
				m_children[i]->queryByScope(r, datas);
		else
			copy(m_datas.begin(), m_datas.end(), back_inserter(datas));
	}
}

//
// class QTree
//

QTree::QTree(const std::vector<IShape*>& shapes, size_t capacity)
{
	build(shapes, capacity);
}

void QTree::queryByScope(const Rect& r, std::vector<QTreeData*>& datas)
{
	if (m_root)
	{
		m_root->queryByScope(r, datas);
		datas.erase(std::unique(datas.begin(), datas.end()), datas.end());
	}
}

void QTree::build(const std::vector<IShape*>& shapes, size_t capacity)
{
	if (shapes.empty())
		return;

	Rect scope;
	shapes.front()->getMBR(scope);
	for (size_t i = 1; i < shapes.size(); ++i)
	{
		Rect tmp;
		shapes[i]->getMBR(tmp);
		scope.combineRect(tmp);
	}

	std::vector<QTreeData*> datas;
	datas.reserve(shapes.size());
	for (size_t i = 0; i < shapes.size(); ++i)
		datas.push_back(new QTreeData(i, shapes[i]));

	m_root = new QTreeNode(scope, datas, capacity);
}