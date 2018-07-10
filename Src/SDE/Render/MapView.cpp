#include "MapView.h"

using namespace IS_SDE;
using namespace IS_SDE::Render;

MapView::MapView()
	: m_screenPixWidth(0), m_screenPixHeight(0)
{}

void MapView::updateViewScope(const MapScope& scope)
{
	m_viewScope.merge(scope);
	setScaleTransform(true);
}

void MapView::resetViewScope(const MapScope& scope)
{
	m_viewScope = scope;
	setScaleTransform(true);
}

void MapView::setScreenPix(size_t width, size_t height)
{
	m_screenPixWidth = width;
	m_screenPixHeight = height;
	setScaleTransform();
}

void MapView::setViewScope(const MapScope& viewScope)
{
	m_viewScope = viewScope;
	setScaleTransform(true);
}

Rect MapView::getViewScope() const
{
	assert(!isNull());
	double low[2], high[2];
	low[0]	= m_viewScope.m_xMin;
	low[1]	= m_viewScope.m_yMin;
	high[0] = m_viewScope.m_xMax;
	high[1] = m_viewScope.m_yMax;

	return Rect(low, high);
}

void MapView::transMapToScreen(const double& s_x, const double& s_y, long* d_x, long* d_y) const
{
	assert(!isNull());
	*d_x = static_cast<long>((s_x - m_viewScope.m_xMin) * m_pixPerDis);
	*d_y = static_cast<long>((m_viewScope.m_yMax - s_y) * m_pixPerDis);
}

void MapView::transScreenToMap(long s_x, long s_y, double* d_x, double* d_y) const
{
	assert(!isNull());
	*d_x = m_viewScope.m_xMin + s_x * m_disPerPix;
	*d_y = m_viewScope.m_yMax - s_y * m_disPerPix;
}

void MapView::transMapToScreen(const MapPos2D& src, ScreenPos2D* dest) const
{
	assert(!isNull());
	dest->d0 = static_cast<long>((src.d0 - m_viewScope.m_xMin) * m_pixPerDis);
	dest->d1 = static_cast<long>((m_viewScope.m_yMax - src.d1) * m_pixPerDis);
}

void MapView::transScreenToMap(const ScreenPos2D& src, MapPos2D* dest) const
{
	assert(!isNull());
	dest->d0 = m_viewScope.m_xMin + src.d0 * m_disPerPix;
	dest->d1 = m_viewScope.m_yMax - src.d1 * m_disPerPix;
}

ScreenPos2D MapView::transMapToScreen(const MapPos2D& src) const
{
	ScreenPos2D ret;
	transMapToScreen(src, &ret);
	return ret;
}

MapPos2D MapView::transScreenToMap(const ScreenPos2D& src) const
{
	MapPos2D ret;
	transScreenToMap(src, &ret);
	return ret;
}

void MapView::transMapToScreen(const std::vector<MapPos2D>& src, std::vector<ScreenPos2D>* dest) const
{
	size_t size = src.size();
	dest->resize(size);
	for (size_t i = 0; i < size; ++i)
		transMapToScreen(src.at(i), &dest->at(i));
}

void MapView::transScreenToMap(const std::vector<ScreenPos2D>& src, std::vector<MapPos2D>* dest) const
{
	size_t size = src.size();
	dest->resize(size);
	for (size_t i = 0; i < size; ++i)
		transScreenToMap(src.at(i), &dest->at(i));
}

void MapView::setScaleTransform(bool center/* = false*/)
{
	if (m_screenPixWidth == 0 || m_screenPixHeight == 0 || m_viewScope.isNull())
		return;

	double x_scale = m_viewScope.xLength() / m_screenPixWidth,
		   y_scale = m_viewScope.yLength() / m_screenPixHeight;

	if (x_scale > y_scale)
	{
		m_disPerPix = x_scale;
		if (center)
		{
			double yHalfLength = (x_scale / y_scale) * m_viewScope.yLength() * 0.5;
			double yCenter = 0.5 * (m_viewScope.m_yMin + m_viewScope.m_yMax);
			m_viewScope.m_yMin = yCenter - yHalfLength;
			m_viewScope.m_yMax = yCenter + yHalfLength;
		}
		else
		{
			m_viewScope.m_yMax = (x_scale / y_scale) * m_viewScope.yLength() + m_viewScope.m_yMin;
		}
	}
	else
	{
		m_disPerPix = y_scale;
		if (center)
		{
			double xHalfLength = (y_scale / x_scale) * m_viewScope.xLength() * 0.5;
			double xCenter = 0.5 * (m_viewScope.m_xMin + m_viewScope.m_xMax);
			m_viewScope.m_xMin = xCenter - xHalfLength;
			m_viewScope.m_xMax = xCenter + xHalfLength;
		}
		else
		{
			m_viewScope.m_xMax = (y_scale / x_scale) * m_viewScope.xLength() + m_viewScope.m_xMin;
		}
	}
	m_pixPerDis = 1 / m_disPerPix;
}

void MapView::zoomOut(long x, long y) 
{
	double xCenter, yCenter;
	transScreenToMap(x, y, &xCenter, &yCenter);

	MapScope scope;
	scope.m_xMin = xCenter - (xCenter - m_viewScope.m_xMin) * 1.1;
	scope.m_xMax = xCenter + (m_viewScope.m_xMax - xCenter) * 1.1;
	scope.m_yMin = yCenter - (yCenter - m_viewScope.m_yMin) * 1.1;
	scope.m_yMax = yCenter + (m_viewScope.m_yMax - yCenter) * 1.1;

	setViewScope(scope);
}

void MapView::zoomIn(long x, long y) 
{
	double xCenter, yCenter;
	transScreenToMap(x, y, &xCenter, &yCenter);

	MapScope scope;
	scope.m_xMin = xCenter - (xCenter - m_viewScope.m_xMin) * 0.9;
	scope.m_xMax = xCenter + (m_viewScope.m_xMax - xCenter) * 0.9;
	scope.m_yMin = yCenter - (yCenter - m_viewScope.m_yMin) * 0.9;
	scope.m_yMax = yCenter + (m_viewScope.m_yMax - yCenter) * 0.9;

	setViewScope(scope);
}

void MapView::move(long dx, long dy)
{
	assert(!isNull());
	double moveDis = movePixCountEachTime * m_disPerPix;
	m_viewScope.m_xMax += dx * moveDis;
	m_viewScope.m_xMin += dx * moveDis;
	m_viewScope.m_yMax += dy * moveDis;
	m_viewScope.m_yMin += dy * moveDis;
}

void MapView::move(long b_x, long b_y, long e_x, long e_y)
{
	assert(!isNull());

	double dx = - (e_x - b_x) * m_disPerPix,
		   dy =   (e_y - b_y) * m_disPerPix;

	MapScope scope;
	scope.m_xMin = m_viewScope.m_xMin + dx;
	scope.m_xMax = m_viewScope.m_xMax + dx;
	scope.m_yMin = m_viewScope.m_yMin + dy;
	scope.m_yMax = m_viewScope.m_yMax + dy;
	setViewScope(scope);
}