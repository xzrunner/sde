#include "MinimalDistance.h"
#include "../../../Algorithms/ComputationalGeometry/DelaunayTriangulation.h"
#include "../../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Measure::Points;

void MinimalDistance::getMeasureValue(Generalization::IAgent* agent, Tools::PropertySet& ps)
{
	Tools::Variant var;

	agent->getAgentProperty("MinimalDistance", var);
	if (var.m_varType != Tools::VT_DOUBLE) throw Tools::IllegalStateException("MinimalDistance::getMeasureValue: Didn't find property.");
	ps.setProperty("MinimalDistance", var);

	agent->getAgentProperty("NearestTwoPoints", var);
	if (var.m_varType != Tools::VT_PVOID) throw Tools::IllegalStateException("MinimalDistance::getMeasureValue: Didn't find property.");
	ps.setProperty("NearestTwoPoints", var);
}

double MinimalDistance::calMinimumDistance(const Algorithms::DelaunayTriangulation* dt,
										   MapPos2D& p0, MapPos2D& p1)
{
	typedef Algorithms::DelaunayTriangulation DT;

	double minDisSquare = Tools::DOUBLE_MAX;

	const std::set<DT::Node*, DT::NodeCmp>& nodes = dt->getAllNodes();
	std::set<DT::Node*, DT::NodeCmp>::const_iterator itr = nodes.begin();
	for ( ; itr != nodes.end(); ++itr)
	{
		MapPos2D curr((*itr)->m_pos);
		std::vector<MapPos2D> surround;
		dt->getSurroundPos(curr, surround);
		for (size_t i = 0; i < surround.size(); ++i)
		{
			double dis = Tools::SpatialMath::getDistanceWithSquare(curr, surround[i]);
			if (dis < minDisSquare)
			{
				minDisSquare = dis;
				p0 = curr;
				p1 = surround[i];
			}
		}
	}

	assert(minDisSquare != Tools::DOUBLE_MAX);
	return sqrt(minDisSquare);
}

double MinimalDistance::calMinimumDistance(const std::vector<MapPos2D>& points,
										   MapPos2D& p0, MapPos2D& p1)
{
	// todo: split-merge

	double minDisSquare = Tools::DOUBLE_MAX;

	for (size_t i = 0; i < points.size() - 1; ++i)
		for (size_t j = i + 1; j < points.size(); ++j)
		{
			double dis = Tools::SpatialMath::getDistanceWithSquare(points[i], points[j]);
			if (dis < minDisSquare)
			{
				minDisSquare = dis;
				p0 = points[i];
				p1 = points[j];
			}
		}

	assert(minDisSquare != Tools::DOUBLE_MAX);
	return sqrt(minDisSquare);
}