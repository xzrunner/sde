#include "KMeansClustering.h"
#include "../Utility/Utility.h"
#include "../../../Tools/SpatialMath.h"
#include "../../../Tools/Random.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

void KMeansClustering::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	std::vector<MapPos2D> srcPos, destPos;
	Utility::getPointsCoords(src, srcPos);

	clustering(srcPos, destPos);

	Utility::createPointsShapes(destPos, dest);
}

void KMeansClustering::clustering(const std::vector<IShape*>& src, std::vector<std::vector<IShape*> >& dest)
{
	std::vector<MapPos2D> srcPos;
	std::vector<std::vector<MapPos2D> > destPos;
	Utility::getPointsCoords(src, srcPos);
	clustering(srcPos, destPos);
	dest.reserve(destPos.size());
	for (size_t i = 0; i < destPos.size(); ++i)
	{
		std::vector<IShape*> tmp;
		Utility::createPointsShapes(destPos[i], tmp);
		dest.push_back(tmp);
	}
}

void KMeansClustering::computerCenter(const CLUSTER_POS& clusterPos, std::vector<MapPos2D>& centers)
{
	centers.clear();
	centers.reserve(clusterPos.size());
	for (size_t i = 0; i < clusterPos.size(); ++i)
	{
		if (clusterPos[i].empty())
			continue;

		MapPos2D center;
		Tools::SpatialMath::getCenterOfPoints(clusterPos[i], center);
		centers.push_back(center);
	}
}

void KMeansClustering::clusterToPos(const std::vector<MapPos2D>& src, 
									const std::vector<MapPos2D>& centers, 
									CLUSTER_POS& clusterPos)
{
	clusterPos.clear();
	if (centers.empty())
		return;

	clusterPos.resize(centers.size());
	for (size_t i = 0; i < src.size(); ++i)
	{
		double minDis = Tools::SpatialMath::getDistanceWithSquare(src[i], centers[0]);
		size_t iNearest = 0;
		for (size_t j = 1; j < centers.size(); ++j)
		{
			double dis = Tools::SpatialMath::getDistanceWithSquare(src[i], centers[j]);
			if (dis < minDis)
			{
				minDis = dis;
				iNearest = j;
			}
		}
		clusterPos[iNearest].push_back(src[i]);
	}
}

void KMeansClustering::clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest)
{
	if (m_k >= src.size())
		return;

	std::vector<MapPos2D> center;
	center.assign(src.begin(), src.end());
	Tools::Random::RandomPermutation(center);
	center.erase(center.begin() + m_k, center.end());
	clusterToPos(src, center, dest);

	while (needRecluster(center, dest))
	{
		computerCenter(dest, center);
		clusterToPos(src, center, dest);
	}
}


void KMeansClustering::clustering(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest) const
{
	if (m_k >= src.size())
	{
		dest.assign(src.begin(), src.end());
		return;
	}

	dest.assign(src.begin(), src.end());
	Tools::Random::RandomPermutation(dest);
	dest.erase(dest.begin() + m_k, dest.end());
	CLUSTER_POS clusterPos;
	clusterToPos(src, dest, clusterPos);

	while (needRecluster(dest, clusterPos))
	{
		computerCenter(clusterPos, dest);
		clusterToPos(src, dest, clusterPos);
	}
}

bool KMeansClustering::needRecluster(const std::vector<MapPos2D>& centers, const CLUSTER_POS& clusterPos) const
{
	assert(centers.size() == clusterPos.size());
	if (centers.empty())
		return false;
	
	for (size_t i = 0; i < centers.size(); ++i)
	{
		assert (clusterPos[i].size() > 0);
		MapPos2D center;
		Tools::SpatialMath::getCenterOfPoints(clusterPos[i], center);
		if (fabs(center.d0 - centers[i].d0) > m_threshold 
			|| fabs(center.d1 - centers[i].d1) > m_threshold)
			return true;
	}

	return false;
}
