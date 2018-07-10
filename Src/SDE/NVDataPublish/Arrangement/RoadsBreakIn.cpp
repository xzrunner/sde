#include "RoadsBreakIn.h"
#include "Point.h"
#include "Math.h"
#include "Line.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE::NVDataPublish::Arrangement;

//////////////////////////////////////////////////////////////////////////
// RoadsBreakIn
//////////////////////////////////////////////////////////////////////////

RoadsBreakIn::RoadsBreakIn(NodeMgr* nodeMgr, const double& precision)
	: m_precision(precision), m_pNodeMgr(nodeMgr), 
	m_eventList(nodeMgr, precision), m_stateList(precision)
{
}

void RoadsBreakIn::handleEventList(double* progress)
{
	if (m_eventList.empty())
		return;

	*progress = 0.0;

	double	tot = m_eventList.high() - m_eventList.low(),
			top = m_eventList.high();

	while(!m_eventList.empty())
	{
		EventPoint p(m_eventList.top()); m_eventList.pop();

		handleEventPoint(p);

		*progress = (top - p.scanningLine()) * 100 / tot;
	}
}

void RoadsBreakIn::handleEventPoint(const EventPoint& p)
{
	Node* n;
	std::vector<Edge*> upIsP, lowIsP;

	if (!p.isIntersect())
	{
		n = p.m_node;
		n->getUpAndLowEdges(&upIsP, &lowIsP);
	}
	else
		n = NULL;

	if (p.m_type == UP)
	{
		if (p.isIntersect())
		{
			MapPos2D realPos(p.m_pos.d0, p.m_pos.d1 - m_precision);
			m_stateList.adjustOrder(realPos, m_precision);
			m_stateList.checkNewEventNode(realPos, &m_eventList);
		}
		else
		{
			m_stateList.updateUpEvent(upIsP, lowIsP, n->pos().d1);
			m_stateList.checkNewEventNode(n, &m_eventList);
		}
	}
	else if (p.m_type == CONTAIN)
	{
		if (p.isIntersect())
		{
			bool bNew = m_pNodeMgr->createNode(p.m_pos, &n);
			if (bNew)
				m_eventList.insertDownEvent(n);
		}

		m_stateList.nodeBreakIn(n, p.m_pos, &m_eventList);
	}
	else // p.m_type == DOWN
	{
		m_stateList.updateDownEvent(lowIsP);
	}
}


//////////////////////////////////////////////////////////////////////////
// RoadsBreakIn::EventPoint
//////////////////////////////////////////////////////////////////////////

RoadsBreakIn::EventPoint::EventPoint(Node* n, EventPointType type, const double& precision)
	: m_node(n), m_pos(n->pos()), m_type(type)
{
	if (type == UP)
		m_pos.d1 += precision;
	else if (type == DOWN)
		m_pos.d1 -= precision;
}

RoadsBreakIn::EventPoint::EventPoint(Node* n, EventPointType type, const MapPos2D& pos)
	: m_node(n), m_pos(pos), m_type(type)
{	
}

double RoadsBreakIn::EventPoint::scanningLine() const
{
	return m_pos.d1;
}

//////////////////////////////////////////////////////////////////////////
// RoadsBreakIn::EventPointComp
//////////////////////////////////////////////////////////////////////////

bool RoadsBreakIn::EventPointComp::operator () (const EventPoint& lhs, const EventPoint& rhs) const
{
	if (lhs.m_pos == rhs.m_pos)
	{
		if (lhs.m_type == rhs.m_type && lhs.isIntersect() == rhs.isIntersect())
		{
			return false;
		}
		else
		{
			assert(lhs.m_type == UP && rhs.m_type == DOWN
				|| lhs.m_type == DOWN && rhs.m_type == UP
				|| lhs.m_type == rhs.m_type && lhs.isIntersect() != rhs.isIntersect());

			if (lhs.m_type == rhs.m_type)
				return lhs.isIntersect();
			else
				return lhs.m_type == DOWN;
		}
	}
	else
	{
		return Math::isPosHigher(lhs.m_pos, rhs.m_pos);
	}
}

//////////////////////////////////////////////////////////////////////////
// Scanning::EventList
//////////////////////////////////////////////////////////////////////////

RoadsBreakIn::EventList::EventList(const NodeMgr* pNodeMgr, const double& precision)
	: m_precision(precision)
{
	const std::map<id_type, std::vector<Node*> >& points = pNodeMgr->getAllData();
	std::map<id_type, std::vector<Node*> >::const_iterator itrGrid = points.begin();
	for ( ; itrGrid != points.end(); ++itrGrid)
	{
		std::vector<Node*>::const_iterator itrPoint = itrGrid->second.begin();
		for ( ; itrPoint != itrGrid->second.end(); ++itrPoint)
			insert(*itrPoint);
	}
}

double RoadsBreakIn::EventList::high() const
{
	return m_list.begin()->m_node->pos().d1;
}

double RoadsBreakIn::EventList::low() const
{
	return (--m_list.end())->m_node->pos().d1;
}

void RoadsBreakIn::EventList::insert(Node* n)
{
	m_list.insert(EventPoint(n, UP, m_precision));
	m_list.insert(EventPoint(n, CONTAIN, m_precision));
	m_list.insert(EventPoint(n, DOWN, m_precision));
}

void RoadsBreakIn::EventList::insert(Node* n, const MapPos2D& pos)
{
	m_list.insert(EventPoint(n, UP, MapPos2D(pos.d0, pos.d1 + m_precision)));
	m_list.insert(EventPoint(n, CONTAIN, pos));
	m_list.insert(EventPoint(n, DOWN, MapPos2D(pos.d0, pos.d1 - m_precision)));
}

void RoadsBreakIn::EventList::insertCross(const MapPos2D& pos)
{
	EventPoint up(NULL, UP, MapPos2D(pos.d0, pos.d1 + m_precision)),
		contain(EventPoint(NULL, CONTAIN, pos));
	if (m_list.find(up) == m_list.end())
		m_list.insert(up);
	if (m_list.find(contain) == m_list.end())
		m_list.insert(contain);
}

void RoadsBreakIn::EventList::insertHor(const MapPos2D& pos)
{
	EventPoint contain(EventPoint(NULL, CONTAIN, pos));
	if (m_list.find(contain) == m_list.end())
		m_list.insert(contain);
}

void RoadsBreakIn::EventList::insertDownEvent(Node* n)
{
	m_list.insert(EventPoint(n, DOWN, m_precision));
}

//////////////////////////////////////////////////////////////////////////
// RoadsBreakIn::StateList
//////////////////////////////////////////////////////////////////////////

RoadsBreakIn::StateList::StateList(const double& precision)
	: m_precision(precision), m_down(precision) 
{
}

void RoadsBreakIn::StateList::adjustOrder(const MapPos2D& p, const double& precision)
{
	std::vector<Edge*> intersectEdges;

	m_down.regionQuery(
		MapPos2D(p.d0 - precision, p.d1),
		MapPos2D(p.d0 + precision, p.d1),
		&intersectEdges
		);

	for (size_t i = 0; i < intersectEdges.size(); ++i)
		m_down.erase(intersectEdges.at(i), p.d1, precision);
	for (size_t i = 0; i < intersectEdges.size(); ++i)
		m_down.insert(intersectEdges.at(i), p.d1);
}

void RoadsBreakIn::StateList::updateUpEvent(
	const std::vector<Edge*>& upIsP, const std::vector<Edge*>& lowIsP, const double& scanLine
	)
{
	for (size_t i = 0; i < upIsP.size(); ++i)
	{
//		m_tot2.insert(upIsP.at(i));
		m_tot.insert(upIsP.at(i));

		if (!upIsP.at(i)->isHorizontal())
			m_down.insert(upIsP.at(i));
	}

	for (size_t i = 0; i < lowIsP.size(); ++i)
	{
		if (!lowIsP.at(i)->isHorizontal())
			m_down.erase(lowIsP.at(i), scanLine, m_precision);
	}
}

void RoadsBreakIn::StateList::updateDownEvent(const std::vector<Edge*>& lowIsP)
{
	for (size_t i = 0; i < lowIsP.size(); ++i)
	{
//		m_tot2.erase(lowIsP.at(i));
		m_tot.erase(lowIsP.at(i));
	}
}

void RoadsBreakIn::StateList::checkNewEventNode(Node* n, EventList* pEventList)
{
	std::vector<Edge*> upIsP, lowIsP;
	n->getUpAndLowEdges(&upIsP, &lowIsP);

	std::vector<Edge*> upIsPNotHor, upIsPHor;
	for (size_t i = 0; i < upIsP.size(); ++i)
	{
		if (!upIsP.at(i)->isHorizontal())
			upIsPNotHor.push_back(upIsP.at(i));
		else
			upIsPHor.push_back(upIsP.at(i));
	}

	for (size_t i = 0; i < upIsPHor.size(); ++i)
	{
		Edge* e = upIsPHor.at(i);

		std::vector<Edge*> checked;
		if (e->beginPos().d0 < e->endPos().d0)
			m_down.regionQuery(
				MapPos2D(e->beginPos().d0, n->pos().d1), 
				MapPos2D(e->endPos().d0, n->pos().d1), 
				&checked
				);
		else
			m_down.regionQuery(
				MapPos2D(e->endPos().d0, n->pos().d1), 
				MapPos2D(e->beginPos().d0, n->pos().d1), 
				&checked
			);

		for (size_t j = 0; j < checked.size(); ++j)
			handleTwoSegsIntersect(e, checked.at(j), n->pos(), pEventList);
	}

	if (upIsPNotHor.empty())
	{
		Edge *left = 0, *right = 0;
		m_down.findNeighbor(n->pos(), &left, &right);
		if (left && right)
			handleTwoSegsIntersect(left, right, n->pos(), pEventList);
	}
	else
	{
		Edge *left = 0, *right = 0, *upLeft = 0, *upRight = 0;
		m_down.findNeighbor(n->pos(), &left, &right);
		m_down.findBoundary(upIsPNotHor, &upLeft, &upRight);

		if (left)
			handleTwoSegsIntersect(left, upLeft, n->pos(), pEventList);
		if (right)
			handleTwoSegsIntersect(upRight, right, n->pos(), pEventList);
	}
}

void RoadsBreakIn::StateList::checkNewEventNode(const MapPos2D& p, EventList* pEventList)
{
	std::vector<Edge*> contain;
	m_down.regionQuery(
		MapPos2D(p.d0 - m_precision, p.d1), 
		MapPos2D(p.d0 + m_precision, p.d1), 
		&contain
		);

	std::vector<Edge*> hasPartUnderP;
	for (size_t i = 0; i < contain.size(); ++i)
	{
		Edge* e = contain.at(i);
		int state = Tools::SpatialMath::testPointOnSection(
			e->beginPos(), e->endPos(), p, m_precision * 2
			);
		assert(state == 0 || state == 1 || state == 2);

		if (state == 1 || state == 2)
		{
			if (Tools::SpatialMath::isTheSamePos(
				e->isBeginHigher() ? e->beginPos() : e->endPos(), p, m_precision
				))
			{
				hasPartUnderP.push_back(e);
			}
		}
		else
			hasPartUnderP.push_back(e);
	}

	if (hasPartUnderP.empty())
	{
		Edge *left = 0, *right = 0;
		m_down.findNeighbor(p, &left, &right);
		if (left && right)
			handleTwoSegsIntersect(left, right, p, pEventList);
	}
	else
	{
		Edge *left = 0, *right = 0, *upLeft = 0, *upRight = 0;
		m_down.findNeighbor(p, &left, &right);
		m_down.findBoundary(hasPartUnderP, &upLeft, &upRight);

		if (left)
			handleTwoSegsIntersect(left, upLeft, p, pEventList);
		if (right)
			handleTwoSegsIntersect(upRight, right, p, pEventList);
	}
}

void RoadsBreakIn::StateList::nodeBreakIn(Node* n, const MapPos2D& real, EventList* pEventList)
{
	std::vector<Edge*> intersectEdges;

	MapScope scope;
	scope.m_xMin = n->pos().d0 - m_precision;
	scope.m_xMax = n->pos().d0 + m_precision;
	scope.m_yMin = n->pos().d1 - m_precision;
	scope.m_yMax = n->pos().d1 + m_precision;

	m_tot.regionQuery(scope, &intersectEdges);

	//std::vector<Edge*> intersectEdges2;
	//m_tot2.regionQuery(scope, &intersectEdges2);
	//// debug
	//if (intersectEdges.size() != intersectEdges2.size())
	//	int zz = 0;
	//if (intersectEdges.size() == intersectEdges2.size())
	//{
	//	std::sort(intersectEdges.begin(), intersectEdges.end());
	//	std::sort(intersectEdges2.begin(), intersectEdges2.end());
	//	for (size_t i = 0; i < intersectEdges.size(); ++i)
	//		assert(intersectEdges.at(i) == intersectEdges2.at(i));
	//}
	////
	
	bool bBreakIn = false;
	for (size_t i = 0; i < intersectEdges.size(); ++i)
	{
		Edge* e = intersectEdges.at(i);
		int state = Tools::SpatialMath::testPointOnSection(e->beginPos(), e->endPos(), n->pos(), m_precision * 2);
		if (state == 1)
			assert(e->beginNode() == n);
		else if (state == 2)
			assert(e->endNode() == n);
		else if (state == 0)
		{
			nodeBreakIn(n, e, real);
			bBreakIn = true;
		}
		assert(state != 3);
	}

	if (bBreakIn)
	{
		std::vector<Edge*> upIsP, lowIsP, test;
		n->getUpAndLowEdges(&upIsP, &lowIsP);
		for (size_t i = 0; i < upIsP.size(); ++i)
		{
			if (!upIsP.at(i)->isHorizontal())
				test.push_back(upIsP.at(i));
		}

		if (!test.empty())
		{
			Edge *left = 0, *right = 0, *upLeft = 0, *upRight = 0;
			m_down.findNeighbor(MapPos2D(n->pos().d0, n->pos().d1 - m_precision), &left, &right);
			m_down.findBoundary(test, &upLeft, &upRight);

			if (left)
				handleTwoSegsIntersect(left, upLeft, n->pos(), pEventList);
			if (right)
				handleTwoSegsIntersect(upRight, right, n->pos(), pEventList);
		}
	}
}

void RoadsBreakIn::StateList::handleTwoSegsIntersect(
	Edge* lhs, Edge* rhs, const MapPos2D& currEvent, EventList* pEventList
	)
{
	MapPos2D crossPoint;
	bool bCross = Tools::SpatialMath::getSegmentCrossPointNoExtension(
		lhs->beginPos(), lhs->endPos(), rhs->beginPos(), rhs->endPos(), &crossPoint, false
		);

	if (bCross)
	{
		if (!Tools::SpatialMath::isTheSamePos(currEvent, crossPoint, m_precision))
		{
			if (Math::isPosHigher(currEvent, crossPoint))
				pEventList->insertCross(crossPoint);
			else if (lhs->isHorizontal() || rhs->isHorizontal())
			{
				if (Math::isPosHigher(MapPos2D(currEvent.d0, currEvent.d1 + m_precision), crossPoint))
					pEventList->insertHor(crossPoint);
			}
		}
	}
}

void RoadsBreakIn::StateList::nodeBreakIn(Node* n, Edge* e, const MapPos2D& real)
{
	m_tot.erase(e);
//	m_tot2.erase(e);
	if (!e->isHorizontal())
	{
		if (Tools::SpatialMath::isBetween(e->beginPos().d1, e->endPos().d1, real.d1 - m_precision))
			m_down.erase(e, real.d1 - m_precision, m_precision);
	}

	Edge* lower = e->cut(n, m_precision);
	e->setDirtyState();
	
	assert(Tools::SpatialMath::isBetween(real.d1 - m_precision, real.d1 + m_precision, n->pos().d1));
	m_tot.insert(lower);
	m_tot.insert(e);

//	m_tot2.insert(lower);
//	m_tot2.insert(e);

	if (!lower->isHorizontal())
		m_down.insert(lower, real.d1 - m_precision);
}