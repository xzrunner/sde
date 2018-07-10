#include "SettlementSpacingRatio.h"
#include "../Line/DouglasPeucker.h"
#include "../Utility/Utility.h"
#include "../Utility/Quadtree.h"
#include "../../../BasicType/Rect.h"
#include "../../../BasicType/Point.h"
#include "../../../BasicType/Line.h"
#include "../../../Tools/SpatialMath.h"
#include "../../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

SettlementSpacingRatio::~SettlementSpacingRatio()
{
	clear();
}

void SettlementSpacingRatio::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	clear();

	std::vector<MapPos2D> srcPos, destPos;
	Utility::getPointsCoords(src, srcPos);

	m_voronoi = new Algorithms::VoronoiByDelaunay(srcPos);
	m_index = new QTree(src, INDEX_CAPACITY);

	selective(srcPos, destPos);

	Utility::createPointsShapes(destPos, dest);
}

void SettlementSpacingRatio::clear()
{
	if (!m_points.empty())
	{
		for_each(m_points.begin(), m_points.end(), Tools::DeletePointerFunctor<SignedPoint>());
		m_points.clear();
	}

	if (m_voronoi)
		delete m_voronoi, m_voronoi = NULL;

	if (m_index)
		delete m_index, m_index = NULL;
}

void SettlementSpacingRatio::selective(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest)
{
	for (size_t i = 0; i < src.size(); ++i)
		if (!m_voronoi->isBoundNode(src[i]))
			m_points.insert(new SignedPoint(src[i], i, m_voronoi->getNodeExtendArea(src[i])));

	std::vector<std::vector<MapPos2D> > boundLinePos;
	m_voronoi->getBoundLinePos(boundLinePos);

	selectBoundLine(boundLinePos, dest);
	m_k -= dest.size();

	std::vector<SignedPoint*> testQueue;
	copy(m_points.begin(), m_points.end(), back_inserter(testQueue));
	sort(testQueue.begin(), testQueue.end(), ImportanceCmp());

	size_t lowerBound(static_cast<size_t>(m_k * 0.9)),
		upperBound(static_cast<size_t>(m_k * 1.1));
	std::vector<SignedPoint*> destSP;
	bool bTested = false;
	double cLower = -1.0, cUpper = -1.0;
	do 
	{
		if (bTested)
		{
			for (size_t i = 0; i < testQueue.size(); ++i)
				testQueue[i]->m_bTested = false;

			if (destSP.size() < lowerBound)
			{
				if (cUpper == -1.0 || m_constant < cUpper)
					cUpper = m_constant;
				m_constant = cLower != -1.0 ? 0.5 * (m_constant + cLower) : 0.5 * m_constant;
			}
			else
			{
				assert(destSP.size() > upperBound);
				if (cLower == -1.0 || m_constant > cLower)
					cLower = m_constant;
				m_constant = cUpper != -1.0 ? 0.5 * (m_constant + cUpper) : 2.0 * m_constant;
			}

			if (!destSP.empty())
				destSP.clear();
		}

		for (size_t i = 0; i < testQueue.size(); ++i)
		{
			SignedPoint* p(testQueue[i]);
			if (isShouldSelected(*p))
				destSP.push_back(p);
			p->m_bTested = true;
		}

		bTested = true;

	} while (destSP.size() < lowerBound || destSP.size() > upperBound);

	dest.reserve(destSP.size());
	for (size_t i = 0; i < destSP.size(); ++i)
		dest.push_back(destSP[i]->m_pos);
}

void SettlementSpacingRatio::selectBoundLine(const std::vector<std::vector<MapPos2D> >& src, std::vector<MapPos2D>& dest) const
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

bool SettlementSpacingRatio::isShouldSelected(const SignedPoint& p) const
{
	double r = m_constant / p.m_importance;

	Rect region(Point(p.m_pos), r);
	std::vector<QTreeData*> tested;
	m_index->queryByScope(region, tested);
	for (size_t i = 0; i < tested.size(); ++i)
	{
		Point* tmp = dynamic_cast<Point*>(tested[i]->m_data);
		assert(tmp);
		MapPos2D testedPos(tmp->m_pCoords[0], tmp->m_pCoords[1]);

		if (Tools::SpatialMath::isTheSamePos(p.m_pos, testedPos, 0.0))
			continue;

		if (Tools::SpatialMath::getDistance(p.m_pos, testedPos) < r)
		{
			SignedPoint tmp(testedPos);
			std::set<SignedPoint*, PosCmp>::const_iterator itr
				= m_points.find(&tmp);
			if (itr != m_points.end() && (*itr)->m_bTested == true)
				return false;
		}
	}

	return true;
}