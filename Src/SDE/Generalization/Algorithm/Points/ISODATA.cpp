#include "ISODATA.h"
#include "KMeansClustering.h"
#include "../Utility/Utility.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

ISODATA::ISODATA(size_t k)
	: m_k(k), m_xThreshold(Tools::DOUBLE_MIN), m_yThreshold(Tools::DOUBLE_MIN)
{
}

ISODATA::ISODATA(double threshold)
	: m_k(Tools::UINT32_MAX), m_xThreshold(threshold), m_yThreshold(threshold)
{
}

ISODATA::ISODATA(size_t k, double threshold)
	: m_k(k), m_xThreshold(threshold), m_yThreshold(threshold)
{
}

ISODATA::~ISODATA()
{
	for_each(m_clusters.begin(), m_clusters.end(), Tools::DeletePointerFunctor<Cluster>());
}

void ISODATA::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	if (!m_clusters.empty())
	{
		for_each(m_clusters.begin(), m_clusters.end(), Tools::DeletePointerFunctor<Cluster>());
		m_clusters.clear();
	}

	std::vector<MapPos2D> srcPos, destPos;
	Utility::getPointsCoords(src, srcPos);

	splitClusters(srcPos);
	mergeClusters(srcPos, destPos);

	Utility::createPointsShapes(destPos, dest);
}

void ISODATA::clustering(const std::vector<IShape*>& src, std::vector<std::vector<IShape*> >& dest)
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

void ISODATA::clustering(const std::vector<MapPos2D>& src, std::vector<std::vector<MapPos2D> >& dest)
{
	if (!m_clusters.empty())
	{
		for_each(m_clusters.begin(), m_clusters.end(), Tools::DeletePointerFunctor<Cluster>());
		m_clusters.clear();
	}

	splitClusters(src);
	std::vector<MapPos2D> centers;
	centers.reserve(m_clusters.size());
	for (size_t i = 0; i < m_clusters.size(); ++i)
		centers.push_back(m_clusters[i]->m_center);
	KMeansClustering::clusterToPos(src, centers, dest);
}

void ISODATA::splitClusters(const std::vector<MapPos2D>& pos)
{
	std::queue<Cluster*> buffer;
	buffer.push(new Cluster(pos));

	while (!buffer.empty())
	{
		Cluster* c = buffer.front(); buffer.pop();
		if (c->m_snx <= m_xThreshold && c->m_sny <= m_yThreshold)
		{
			m_clusters.push_back(c);
			if (m_clusters.size() >= m_k)
				break;
			else
				continue;
		}
		else
		{
			MapPos2D c0(c->m_center), c1(c->m_center);
			bool bDividX;
			if (c->m_snx > m_xThreshold && c->m_sny > m_yThreshold)
				bDividX = c->m_snx > c->m_sny ? true : false;
			else if (c->m_snx > m_xThreshold)
				bDividX = true;
			else
				bDividX = false;
			if (bDividX)
				c0.d0 -= c->m_snx, c1.d0 += c->m_snx;
			else
				c0.d1 -= c->m_sny, c1.d1 += c->m_sny;

			Cluster *cluster0 = new Cluster,
				*cluster1 = new Cluster;

			splitPos(c->m_pos, c0, c1, cluster0->m_pos, cluster1->m_pos);
			cluster0->init();
			cluster1->init();

			buffer.push(cluster0);
			buffer.push(cluster1);
			delete c;
		}
	}
}

void ISODATA::mergeClusters(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest) const
{
	std::vector<MapPos2D> centers;
	centers.reserve(m_clusters.size());
	for (size_t i = 0; i < m_clusters.size(); ++i)
		centers.push_back(m_clusters[i]->m_center);

	std::vector<std::vector<MapPos2D> > clusters;
	KMeansClustering::clusterToPos(src, centers, clusters);
	KMeansClustering::computerCenter(clusters, dest);
}

void ISODATA::calStandardDeviation(const std::vector<MapPos2D>& src, MapPos2D* center,
								   double* snx, double* sny)
{
	size_t count = src.size();

	center->d0 = center->d1 = 0;
	for (size_t i = 0; i < count; ++i)
	{
		center->d0 += src[i].d0;
		center->d1 += src[i].d1;
	}
	center->d0 /= count;
	center->d1 /= count;

	double xSum = 0, ySum = 0;
	for (size_t i = 0; i < count; ++i)
	{
		xSum += (src[i].d0 - center->d0) * (src[i].d0 - center->d0);
		ySum += (src[i].d1 - center->d1) * (src[i].d1 - center->d1);
	}
	
	*snx = sqrt(xSum / count);
	*sny = sqrt(ySum / count);
}

void ISODATA::splitPos(const std::vector<MapPos2D>& src, const MapPos2D& c0, const MapPos2D& c1, 
					   std::vector<MapPos2D>& dest0, std::vector<MapPos2D>& dest1)
{
	std::vector<MapPos2D> centers;
	std::vector<std::vector<MapPos2D> > result;
	centers.push_back(c0);
	centers.push_back(c1);
	KMeansClustering::clusterToPos(src, centers, result);
	dest0 = result[0];
	dest1 = result[1];
}