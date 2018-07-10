#include "DouglasPeucker.h"
#include "../../../BasicType/Line.h"
#include "../../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

void DouglasPeucker::implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest)
{
	dest.clear();
	dest.reserve(src.size());
	for (size_t i = 0; i < src.size(); ++i)
	{
		Line* l = dynamic_cast<Line*>(src[i]);
		assert(l);
		Line* result = new Line;
		implement(l, result);
		dest.push_back(result);
	}
}

void DouglasPeucker::implement(const Line* src, Line* dest)
{
	initMiddleData(src);
	pointReduction(0, src->m_posNum - 1);
	getResult(dest);
}

void DouglasPeucker::initMiddleData(const Line* l)
{
	m_tmpPos.clear();
	m_tmpPos.reserve(l->m_posNum);
	for (size_t i = 0; i < l->m_posNum; ++i)
	{
		MapPos2D p(l->m_pCoords[i * 2], l->m_pCoords[i * 2 + 1]);
		m_tmpPos.push_back(std::make_pair(p, false));
	}
}

void DouglasPeucker::pointReduction(size_t begin, size_t end)
{
	if ( begin > end )
		return;

	if ( begin == end )
	{
		m_tmpPos[begin].second = true;
		return;
	}

	if ( end - begin < 2 )
	{
		m_tmpPos[begin].second = true;
		m_tmpPos[end].second = true;
		return;
	}

	size_t savedPointIndex = 0;
	double max = 0.0;
	for (size_t i = begin + 1; i < end; ++i)
	{
		double dis = Tools::SpatialMath::getDisPointToSegment(m_tmpPos[i].first, m_tmpPos[begin].first, m_tmpPos[end].first);
		if (dis > max)
		{
			max = dis;
			savedPointIndex = i;
		}
	}

	if (max < m_precision)
	{
		m_tmpPos[begin].second = true;
		m_tmpPos[end].second = true;
	}
	else
	{
		pointReduction(begin, savedPointIndex);
		pointReduction(savedPointIndex, end);
	}
}

void DouglasPeucker::getResult(Line* l) const
{
	l->m_posNum = 0;
	for (size_t i = 0; i < m_tmpPos.size(); ++i)
		if (m_tmpPos[i].second)
			++l->m_posNum;

	size_t pos = 0;
	delete[] l->m_pCoords;
	l->m_pCoords = new double[l->m_posNum * l->DIMENSION];
	for (size_t i = 0; i < m_tmpPos.size(); ++i)
		if (m_tmpPos[i].second)
		{
			l->m_pCoords[pos++] = m_tmpPos[i].first.d0;
			l->m_pCoords[pos++] = m_tmpPos[i].first.d1;
		}
}