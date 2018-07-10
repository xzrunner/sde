#include "ScanningLine.h"
#include "Line.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE::NVDataPublish::Arrangement;

//////////////////////////////////////////////////////////////////////////
// ScanningLine
//////////////////////////////////////////////////////////////////////////

ScanningLine::ScanningLine(const double& precision)
	: m_precision(precision), m_list(SegCmp(precision))
{
}

void ScanningLine::insert(Edge* e)
{
	m_list.insert(Segment(e));
}

void ScanningLine::insert(Edge* e, const double& scanLine)
{
	m_list.insert(Segment(e, scanLine));
}

void ScanningLine::erase(Edge* e, const double& scanLine, const double& precision)
{
	assert(Tools::SpatialMath::isBetween(e->beginPos().d1, e->endPos().d1, scanLine));

	MapPos2D low;
	low.d0 = Tools::SpatialMath::findXOnSeg(e->beginPos(), e->endPos(), scanLine);
	low.d1 = scanLine;

	MapPos2D left(MapPos2D(low.d0 - precision, low.d1)), right(MapPos2D(low.d0 + precision, low.d1));

	SCANNING_LIST::iterator itrFind;
	SCANNING_LIST::iterator 
		lower = m_list.lower_bound(Segment(left)),
		upper = m_list.lower_bound(Segment(right));

	for ( ; lower != upper; ++lower)
	{
		if (lower->m_edge == e)
		{
			m_list.erase(lower);
			return;
		}
	}

	SCANNING_LIST::iterator itr = m_list.begin();
	for ( ; itr != m_list.end(); ++itr)
	{
		if (itr->m_edge == e)
		{
			m_list.erase(itr);
			return;
		}
	}
}

void ScanningLine::regionQuery(
	const MapPos2D& left, const MapPos2D& right, std::vector<Edge*>* ret
	) const
{
	SCANNING_LIST::const_iterator 
		lower = m_list.lower_bound(Segment(left)),
		upper = m_list.lower_bound(Segment(right));

	for ( ; lower != upper; ++lower)
		ret->push_back(lower->m_edge);
}

void ScanningLine::findNeighbor(
	const MapPos2D& p, Edge** left, Edge** right
	) const
{
	if (!m_list.empty())
	{
		Segment search(p);
		SCANNING_LIST::const_iterator center = m_list.lower_bound(search),
			itrTmp = center;

		if (itrTmp != m_list.end())
		{
			do 
			{
				int state = SegCmp::compPointWithLine(p, *itrTmp->m_edge, m_precision);
				if (state == -1)
				{
					*right = itrTmp->m_edge;
					break;
				}
				++itrTmp;
			} while (itrTmp != m_list.end());
		}

		itrTmp = center;
		while (itrTmp != m_list.begin())
		{
			--itrTmp;

			int state = SegCmp::compPointWithLine(p, *itrTmp->m_edge, m_precision);
			if (state == 1)
			{
				*left = itrTmp->m_edge;
				break;
			}
		}
	}
}

void ScanningLine::findBoundary(
	const std::vector<Edge*>& up, Edge** left, Edge** right
	) const
{
	*left = up.front();
	*right = *left;
	for (size_t i = 1; i < up.size(); ++i)
	{
		if (up.at(i)->getSlope() < (*left)->getSlope())
			*left = up.at(i);
		if ((*right)->getSlope() < up.at(i)->getSlope())
			*right = up.at(i);
	}
}

//////////////////////////////////////////////////////////////////////////
// ScanningLine::Segment
//////////////////////////////////////////////////////////////////////////

ScanningLine::Segment::Segment(Edge* e)
	: m_edge(e), m_bPoint(false)
{
	m_compPos = e->isBeginHigher() ? e->beginPos() : e->endPos();
}

ScanningLine::Segment::Segment(Edge* e, const double& scanLine)
	: m_edge(e), m_bPoint(false)
{
	assert(Tools::SpatialMath::isBetween(e->beginPos().d1, e->endPos().d1, scanLine));
	m_compPos.d0 = Tools::SpatialMath::findXOnSeg(e->beginPos(), e->endPos(), scanLine);
	m_compPos.d1 = scanLine;
}

ScanningLine::Segment::Segment(const MapPos2D& p)
	: m_edge(NULL), m_compPos(p), m_bPoint(true)
{
}

//////////////////////////////////////////////////////////////////////////
// ScanningLine::SegCmp
//////////////////////////////////////////////////////////////////////////

ScanningLine::SegCmp::SegCmp(const double& precision)
	: m_precision(precision / 2)
{
}

bool ScanningLine::SegCmp::operator () (
	const Segment& lhs, const Segment& rhs
	) const
{
	if (lhs.m_bPoint || rhs.m_bPoint)
	{
		assert(!lhs.m_bPoint || !rhs.m_bPoint);
		if (lhs.m_bPoint)
			return compPointWithLine(lhs.m_compPos, *rhs.m_edge, m_precision) == -1;
		else
			return compPointWithLine(rhs.m_compPos, *lhs.m_edge, m_precision) == 1;
	}
	else
	{
		return compLineWithLine(lhs, rhs, m_precision) == -1;
	}
}

int ScanningLine::SegCmp::compPointWithLine(
	const MapPos2D& p, const Edge& l, const double& precision
	)
{
	// Rough test 
	double	lxMin = l.beginPos().d0, 
			lxMax = l.endPos().d0;
	if (lxMin > lxMax)
	{
		lxMin = l.endPos().d0;
		lxMax = l.beginPos().d0;
	}
	if (p.d0 < lxMin)
		return -1;
	if (p.d0 > lxMax)
		return 1;

	// fine test
	assert(Tools::SpatialMath::isBetween(l.beginPos().d1, l.endPos().d1, p.d1));

	double cmpX = Tools::SpatialMath::findXOnSeg(l.beginPos(), l.endPos(), p.d1);
	if (fabs(cmpX - p.d0) < precision)
		return 0;
	if (cmpX < p.d0 - precision)
		return 1;
	else
		return -1;
}

int ScanningLine::SegCmp::compLineWithLine(
	const Segment& lhs, const Segment& rhs, const double& precision
	)
{
	// Coherence the compPos
	MapPos2D lComp = lhs.m_compPos,
		rComp = rhs.m_compPos;
	if (lComp.d1 > rComp.d1)
	{
		assert(Tools::SpatialMath::isBetween(
			lhs.m_edge->beginPos().d1, lhs.m_edge->endPos().d1, rComp.d1
			));

		lComp.d0 = Tools::SpatialMath::findXOnSeg(lhs.m_edge->beginPos(), lhs.m_edge->endPos(), rComp.d1);
		lComp.d1 = rComp.d1;
	}
	else if (rComp.d1 > lComp.d1)
	{
		assert(Tools::SpatialMath::isBetween(
			rhs.m_edge->beginPos().d1, rhs.m_edge->endPos().d1, lComp.d1
			));

		rComp.d0 = Tools::SpatialMath::findXOnSeg(rhs.m_edge->beginPos(), rhs.m_edge->endPos(), lComp.d1);
		rComp.d1 = lComp.d1;
	}

	// Not the same position
	if (fabs(lComp.d0 - rComp.d0) > precision)
	{
		if (lComp.d0 < rComp.d0)
			return -1;
		else
			return 1;
	}
	// Same position, compare slope
	else
	{
		double slopeDifference = lhs.m_edge->getSlope() - rhs.m_edge->getSlope();
		if (fabs(slopeDifference) < Tools::DOUBLE_TOLERANCE)
			return 0;
		else
		{
			if (slopeDifference > 0)
				return 1;
			else
				return -1;
		}
	}
}
