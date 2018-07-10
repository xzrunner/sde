#include "AlphaShape.h"
#include "ISODATA.h"
#include "../Utility/Utility.h"
#include "../../../Algorithms/ComputationalGeometry/DelaunayTriangulation.h"
#include "../../../BasicType/Line.h"
#include "../../../BasicType/Point.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

AlphaShape::AlphaShape(size_t k)
{
	m_clusterAlgo = new ISODATA(k, Tools::DOUBLE_ZERO);
}

AlphaShape::~AlphaShape()
{
	delete m_clusterAlgo;
}

void AlphaShape::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	std::vector<MapPos2D> srcPos;
	Utility::getPointsCoords(src, srcPos);

	std::vector<std::vector<MapPos2D> > cluster;
	clustering(srcPos, cluster);

	for (size_t i = 0; i < cluster.size(); ++i)
	{
		if (cluster[i].size() < 3)
		{
			if (cluster[i].size() == 1)
				dest.push_back(new Point(cluster[i][0]));
			else if (cluster[i].size() == 2)
				dest.push_back(new Line(cluster[i]));
		}
		else
		{
			Algorithms::DelaunayTriangulation dt(cluster[i]);
			std::vector<std::vector<MapPos2D> > boundLines;
			dt.getBoundLinePos(boundLines);
			for (size_t j = 0; j < boundLines.size(); ++j)
			{
				boundLines[j].push_back(boundLines[j].front());
				dest.push_back(new Line(boundLines[j]));
			}
		}
	}
}

void AlphaShape::clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest) const
{
	std::vector<IShape*> srcShapes;
	std::vector<std::vector<IShape*> > destShapes;
	Utility::createPointsShapes(src, srcShapes);
	m_clusterAlgo->clustering(srcShapes, destShapes);
	dest.reserve(destShapes.size());
	for (size_t i = 0; i < destShapes.size(); ++i)
	{
		std::vector<MapPos2D> tmp;
		Utility::getPointsCoords(destShapes[i], tmp);
		dest.push_back(tmp);
	}
}