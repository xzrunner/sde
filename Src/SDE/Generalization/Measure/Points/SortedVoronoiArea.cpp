#include "SortedVoronoiArea.h"
#include "../../../Algorithms/ComputationalGeometry/VoronoiByDelaunay.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization;
using namespace IS_SDE::Generalization::Measure::Points;

void SortedVoronoiArea::getMeasureValue(IAgent* agent, Tools::PropertySet& ps)
{
	Tools::Variant var;

	agent->getAgentProperty("SortedVoronoiArea", var);
	if (var.m_varType != Tools::VT_DOUBLE) throw Tools::IllegalStateException("SortedVoronoiArea::getMeasureValue: Didn't find property.");
	ps.setProperty("SortedVoronoiArea", var);
}

void SortedVoronoiArea::calSortedVoronoiArea(Algorithms::VoronoiByDelaunay* voronoi, std::vector<double>& areaList)
{
	std::vector<MapPos2D> allPos;
	voronoi->getAllNodePos(allPos);

	std::vector<MapPos2D> centerPos;
	centerPos.reserve(allPos.size());
	for (size_t i = 0; i < allPos.size(); ++i)
		if (!voronoi->isBoundNode(allPos[i]))
			centerPos.push_back(allPos[i]);

	std::vector<double> areaVct;
	areaVct.reserve(centerPos.size());
	for (size_t i = 0; i < centerPos.size(); ++i)
	{
		double face = voronoi->getNodeExtendArea(centerPos[i]);
		assert(face);
		areaList.push_back(face);
	}

	sort(areaList.begin(), areaList.end(), std::less<double>());
}