#include "StructuralSimplification.h"
#include "../Line/DouglasPeucker.h"
#include "../Utility/Utility.h"
#include "../../../BasicType/Line.h"
#include "../../../Tools/SpatialMath.h"
#include "../../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"
#include "../../Measure/Points/DensityConsistency.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

StructuralSimplification::StructuralSimplification(size_t k, size_t forbidDistance/* = 1*/)
	: m_k(k), m_forbidDistance(forbidDistance), m_voronoi(NULL)
{
}

StructuralSimplification::~StructuralSimplification()
{
	clear();
}

void StructuralSimplification::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	clear();

	std::vector<MapPos2D> srcPos, destPos;
	Utility::getPointsCoords(src, srcPos);

	m_voronoi = new Algorithms::VoronoiByDelaunay(srcPos);
	selective(srcPos, destPos);

	Utility::createPointsShapes(destPos, dest);
}

//
// class StructuralSimplification::RandomSelective
//

StructuralSimplification::RandomSelective::~RandomSelective()
{
	clear();
}

void StructuralSimplification::RandomSelective::
	initSelectedData(const std::vector<MapPos2D>& pos, const std::vector<double>& impt)
{
	clear();

	assert(pos.size() == impt.size() && !pos.empty());
	for (size_t i = 0; i < pos.size(); ++i)
		m_data.push_back(new PosWithImportance(pos[i], impt[i]));

	reConstructStartPos();
}

bool StructuralSimplification::RandomSelective::select(MapPos2D& pos)
{
	if (m_data.empty())
		return false;

	srand((unsigned)time(NULL));
	double startPos = 1.0 * rand() / (RAND_MAX + 1.0);
	PosWithImportance findPos(startPos);
	
	std::vector<PosWithImportance*>::iterator itr
		= lower_bound(m_data.begin(), m_data.end(), &findPos, PositionCmp());
	if (itr != m_data.begin())
		--itr;

	pos = (*itr)->m_pos;
	delete *itr;
	m_data.erase(itr);
	if (!m_data.empty())
		reConstructStartPos();

	return true;
}

void StructuralSimplification::RandomSelective::clear()
{
	if (!m_data.empty())
	{
		for_each(m_data.begin(), m_data.end(), Tools::DeletePointerFunctor<PosWithImportance>());
		m_data.clear();
	}
}

void StructuralSimplification::RandomSelective::reConstructStartPos()
{
	double sumImpt = 0;
	for (size_t i = 0; i < m_data.size(); ++i)
		sumImpt += m_data[i]->m_importance;

	m_data[0]->m_start = 0;
	for (size_t i = 1; i < m_data.size(); ++i)
		m_data[i]->m_start = m_data[i - 1]->m_start + m_data[i]->m_importance / sumImpt;
}

//
// class StructuralSimplification::SelectStrategy
//

StructuralSimplification::SelectStrategy::SelectStrategy(Algorithms::VoronoiByDelaunay* voronoi, 
														 size_t k, size_t forbidDistance)
	: m_voronoi(voronoi), m_k(k), m_forbidDistance(forbidDistance)
{
}

void StructuralSimplification::SelectStrategy::
	updateForbiddenSet(const MapPos2D& p, std::set<MapPos2D, PosCmp<double> >& forbidden) const
{
	std::set<MapPos2D, PosCmp<double> > lastCenter, currCenter;
	lastCenter.insert(p);
	for (size_t i = 0; i < m_forbidDistance; ++i)
	{
		std::set<MapPos2D, PosCmp<double> >::iterator itr = lastCenter.begin();
		for ( ; itr != lastCenter.end(); ++itr)
		{
			std::vector<MapPos2D> surround;
			m_voronoi->getSurroundPos(p, surround);
			for (size_t i = 0; i < surround.size(); ++i)
			{
				currCenter.insert(surround[i]);
				forbidden.insert(surround[i]);
			}
		}

		lastCenter = currCenter;
		currCenter.clear();
	}
}

StructuralSimplification::OrderSelectStrategy::OrderSelectStrategy(
	Algorithms::VoronoiByDelaunay* voronoi, size_t k, size_t forbidDistance)
	: SelectStrategy(voronoi, k, forbidDistance)
{
}

void StructuralSimplification::OrderSelectStrategy::select(
	std::set<MapPos2D, PosCmp<double> >& points, std::vector<MapPos2D>& dest) const
{
	std::set<MapPos2D, PosCmp<double> > forbidden;
	std::multimap<double, MapPos2D, std::greater<double> > mapImptPos;

	std::set<MapPos2D, PosCmp<double> >::iterator itrSrc = points.begin();
	for ( ; itrSrc != points.end(); ++itrSrc)
	{
		double impt = 1 / m_voronoi->getNodeExtendArea(*itrSrc);
		mapImptPos.insert(std::make_pair(impt, *itrSrc));
	}

	std::vector<MapPos2D> deletedPos;

	std::multimap<double, MapPos2D, std::greater<double> >::iterator itrImpt 
		= mapImptPos.begin();
	for ( ; itrImpt != mapImptPos.end(); ++itrImpt)
	{
		if (forbidden.find(itrImpt->second) != forbidden.end())
			continue;
		else
		{
			updateForbiddenSet(itrImpt->second, forbidden);

			dest.push_back(itrImpt->second);
			deletedPos.push_back(itrImpt->second);
			points.erase(itrImpt->second);

			if (dest.size() >= m_k)
				break;
		}
	}

	m_voronoi->deleteNodes(deletedPos);
}

StructuralSimplification::RandomSelectStrategy::RandomSelectStrategy(
	Algorithms::VoronoiByDelaunay* voronoi, size_t k, size_t forbidDistance)
	: SelectStrategy(voronoi, k, forbidDistance)
{
}

void StructuralSimplification::RandomSelectStrategy::select(
	std::set<MapPos2D, PosCmp<double> >& points, std::vector<MapPos2D>& dest) const
{
	std::vector<MapPos2D> posVct;
	std::vector<double> imptVct;
	posVct.reserve(points.size());
	imptVct.reserve(points.size());

	std::set<MapPos2D, PosCmp<double> >::iterator itrSrc = points.begin();
	for ( ; itrSrc != points.end(); ++itrSrc)
	{
		double impt = 1 / m_voronoi->getNodeExtendArea(*itrSrc);
		posVct.push_back(*itrSrc);
		imptVct.push_back(impt);
	}

	RandomSelective rs;
	rs.initSelectedData(posVct, imptVct);

	std::set<MapPos2D, PosCmp<double> > forbidden;
	std::vector<MapPos2D> deletedPos;
	MapPos2D chose;
	while (rs.select(chose))
	{
		if (forbidden.find(chose) != forbidden.end())
			continue;
		else
		{
			updateForbiddenSet(chose, forbidden);

			dest.push_back(chose);
			deletedPos.push_back(chose);
			points.erase(chose);

			if (dest.size() >= m_k)
				break;
		}
	}

	m_voronoi->deleteNodes(deletedPos);
}

//
// Internal
//

void StructuralSimplification::clear()
{
	delete m_voronoi;
}

void StructuralSimplification::selective(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest)
{
	for (size_t i = 0; i < src.size(); ++i)
		if (!m_voronoi->isBoundNode(src[i]))
			m_points.insert(src[i]);

	std::vector<std::vector<MapPos2D> > boundLinePos;
	m_voronoi->getBoundLinePos(boundLinePos);

	selectBoundLine(boundLinePos, dest);

//	SelectStrategy* ss = new OrderSelectStrategy(m_voronoi, m_k, m_forbidDistance);
	SelectStrategy* ss = new RandomSelectStrategy(m_voronoi, m_k, m_forbidDistance);
	while (dest.size() < m_k && !m_points.empty())
		ss->select(m_points, dest);
	delete ss;
}

void StructuralSimplification::selectBoundLine(const std::vector<std::vector<MapPos2D> >& src, std::vector<MapPos2D>& dest) const
{
	for (size_t i = 0; i < src.size(); ++i)
	{
		std::vector<MapPos2D> line(src[i]);

		if (line.size() <= 3)
			copy(line.begin(), line.end(), back_inserter(dest));
		else
		{
			line.push_back(line.front());

			double dis = 0;
			for (size_t i = 0; i < line.size() - 1; ++i)
				dis += Tools::SpatialMath::getDistance(line[i], line[i + 1]);
			dis /= (line.size() - 1);

			DouglasPeucker DP(dis * 0.5);

			std::vector<IShape*> srcShape, destShape;
			srcShape.push_back(new Line(line));
			DP.implement(srcShape, destShape);
			Line* l = dynamic_cast<Line*>(destShape.front());
			assert(l);
			l->getPoint(&dest);
			delete srcShape.front();
		}
	}
}

