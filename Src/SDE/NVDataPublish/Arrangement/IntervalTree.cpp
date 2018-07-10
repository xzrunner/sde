#include "IntervalTree.h"
#include "Line.h"
#include "Math.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE::NVDataPublish::Arrangement;

//
// IntervalTree
//

void IntervalTree::insert(Edge* e)
{
	if (Math::isPosLefter(e->beginPos(), e->endPos()))
	{
		m_mapLeftCoords.insert(std::make_pair(e->beginPos().d0, e));
	}
	else
	{
		m_mapLeftCoords.insert(std::make_pair(e->endPos().d0, e));
	}
}

void IntervalTree::erase(Edge* e)
{
	double left, right;
	if (e->beginPos().d0 < e->endPos().d0)
	{
		left = e->beginPos().d0;
		right = e->endPos().d0;
	}
	else
	{
		left = e->endPos().d0;
		right = e->beginPos().d0;
	}

	HOR_LIST_RANGE region = m_mapLeftCoords.equal_range(left);
	assert(region.first != region.second);
	assert(region.first != m_mapLeftCoords.end());

	HOR_LIST::iterator itr = region.first;
	for ( ; itr != region.second; ++itr)
	{
		if (itr->second == e)
			break;
	}
	assert(itr != region.second);
	m_mapLeftCoords.erase(itr);
}

void IntervalTree::regionQuery(const MapScope& scope, std::vector<Edge*>* ret) const
{
	if (m_mapLeftCoords.empty())
		return;

	HOR_LIST::const_iterator itrRBound = m_mapLeftCoords.upper_bound(scope.m_xMax);

	for (HOR_LIST::const_iterator itr = m_mapLeftCoords.begin(); itr != itrRBound; ++itr)
	{
		Edge* e = itr->second;

		double leftEdge, rightEdge;
		if (e->beginPos().d0 < e->endPos().d0)
		{
			leftEdge = e->beginPos().d0;
			rightEdge = e->endPos().d0;
		}
		else
		{
			leftEdge = e->endPos().d0;
			rightEdge = e->beginPos().d0;
		}

		if (rightEdge > scope.m_xMin && leftEdge < scope.m_xMax)
		{
			if (Tools::SpatialMath::isSegmentIntersectRect(e->beginPos(), e->endPos(), scope))
				ret->push_back(e);
		}
	}
}

//
//	SegmentTree
//

void SegmentTree::insert(Edge* e)
{
	bool isBeginLeft = Math::isPosLefter(e->beginPos(), e->endPos());
	Node left(isBeginLeft ? e->beginPos() : e->endPos(), LEFT, e),
		right(isBeginLeft ? e->endPos() : e->beginPos(), RIGHT, e);

	ITR_LIST lLeft, rLeft, lRight, rRight;
	findPosition(left, lLeft, rLeft);
	findPosition(right, lRight, rRight);

	getThroughEdges(lLeft, &left.m_through);
	getThroughEdges(lRight, &right.m_through);

	for (ITR_LIST itr = lLeft == m_list.end() ? m_list.begin() : lLeft; itr != rRight; ++itr)
	{
		if (itr->m_belongTo == e)
			continue;

		if (Math::isPosLefter(left.m_pos, itr->m_pos) &&
			Math::isPosLefter(itr->m_pos, right.m_pos))
		{
			itr->m_through.insert(e);
		}
	}

	m_list.insert(left);
	m_list.insert(right);
}

void SegmentTree::erase(Edge* e)
{
	bool isBeginLeft = Math::isPosLefter(e->beginPos(), e->endPos());
	Node left(isBeginLeft ? e->beginPos() : e->endPos(), LEFT, e),
		right(isBeginLeft ? e->endPos() : e->beginPos(), RIGHT, e);

	ITR_LIST lLeft, rLeft, lRight, rRight;
	findPosition(left, lLeft, rLeft);
	findPosition(right, lRight, rRight);

	for (ITR_LIST itr = lLeft == m_list.end() ? m_list.begin() : lLeft; itr != rRight; ++itr)
	{
		if (itr->m_belongTo == e)
			continue;

		if (Math::isPosLefter(left.m_pos, itr->m_pos) &&
			Math::isPosLefter(itr->m_pos, right.m_pos))
		{
			std::set<Edge*>::iterator find = itr->m_through.find(e);
			assert(find != itr->m_through.end());
			itr->m_through.erase(find);
		}
	}

	ITR_LIST itrLeft = m_list.find(left),
		itrRight = m_list.find(right);
	assert(itrLeft != m_list.end() && itrRight != m_list.end());
	m_list.erase(itrLeft);
	m_list.erase(itrRight);
}

void SegmentTree::regionQuery(const MapScope& scope, std::vector<Edge*>* ret)
{
	double halfHeight = 0.5 * (scope.m_yMax - scope.m_yMin);
	Node left(MapPos2D(scope.m_xMin, scope.m_yMin + halfHeight)),
		right(MapPos2D(scope.m_xMax, scope.m_yMin + halfHeight));

	ITR_LIST lLeft, rLeft, lRight, rRight;
	findPosition(left, lLeft, rLeft);
	findPosition(right, lRight, rRight);

	std::set<Edge*> shouldChecked;
	getThroughEdges(lLeft, &shouldChecked);

	for (ITR_LIST itr = lLeft == m_list.end() ? m_list.begin() : lLeft; itr != rRight; ++itr)
	{
		if (itr->m_type == LEFT)
		{
			shouldChecked.insert(itr->m_belongTo);
		}
	}

	std::set<Edge*>::iterator itrCheck = shouldChecked.begin();
	for ( ; itrCheck != shouldChecked.end(); ++itrCheck)
	{
		double leftEdge, rightEdge;
		if ((*itrCheck)->beginPos().d0 < (*itrCheck)->endPos().d0)
		{
			leftEdge = (*itrCheck)->beginPos().d0;
			rightEdge = (*itrCheck)->endPos().d0;
		}
		else
		{
			leftEdge = (*itrCheck)->endPos().d0;
			rightEdge = (*itrCheck)->beginPos().d0;
		}

		if (rightEdge > scope.m_xMin && leftEdge < scope.m_xMax)
		{
			if (Tools::SpatialMath::isSegmentIntersectRect((*itrCheck)->beginPos(), (*itrCheck)->endPos(), scope))
				ret->push_back(*itrCheck);
		}
	}
}

bool SegmentTree::NodeCmp::operator () (const Node& lhs, const Node& rhs) const
{
	if (lhs.m_pos == rhs.m_pos)
	{
		if (lhs.m_belongTo == rhs.m_belongTo)
		{
			if (lhs.m_type == rhs.m_type)
				return false;
			else
				return lhs.m_type == LEFT;
		}
		else
		{
			return lhs.m_belongTo < rhs.m_belongTo;
		}
	}
	else
	{
		return Math::isPosLefter(lhs.m_pos, rhs.m_pos);
	}
}

void SegmentTree::findPosition(const Node& n, ITR_LIST& left, ITR_LIST& right)
{
	if (m_list.empty())
	{
		left = m_list.end();
		right = m_list.end();
	}
	else
	{
		right = m_list.upper_bound(n);
		while (right != m_list.end() 
			&& !Math::isPosLefter(n.m_pos, right->m_pos))
		{
			++right;
			if (right == m_list.end())
				break;
		}

		ITR_LIST tmp = right;
		if (tmp != m_list.end())
		{
			while (++tmp != m_list.end())
			{
				if (tmp->m_pos == right->m_pos)
					right = tmp;
				else
					break;
			}
		}

		left = right;
		bool bSucc = false;
		while (left != m_list.begin())
		{
			--left;
			if (Math::isPosLefter(left->m_pos, n.m_pos))
			{
				bSucc = true;
				break;
			}
		}
		
		if (!bSucc)
			left = m_list.end();
		else
		{
			tmp = left;
			while (tmp != m_list.begin())
			{
				--tmp;
				if (tmp->m_pos == left->m_pos)
					left = tmp;
				else
					break;
			}
		}
	}
}

void SegmentTree::getThroughEdges(ITR_LIST& p, std::set<Edge*>* result) const
{
	if (p == m_list.end())
		return;

	ITR_LIST tmp(p);
	if (tmp != m_list.begin())
	{
		--tmp;
		assert(tmp->m_pos != p->m_pos);
		++tmp;
	}

	do 
	{
		std::set<Edge*>::iterator itr = tmp->m_through.begin();
		for ( ; itr != tmp->m_through.end(); ++itr)
			result->insert(*itr);
		if (tmp->m_type == LEFT)
			result->insert(tmp->m_belongTo);
		
		++tmp;

	} while (tmp != m_list.end() && tmp->m_pos == p->m_pos);
}