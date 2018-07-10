#include "Render.h"
#include "MapView.h"
#include "../BasicType/Rect.h"
#include "../BasicType/Point.h"
#include "../BasicType/Line.h"
//#include "../BasicType/Polygon.h"
#include "Selection.h"

using namespace IS_SDE;
using namespace IS_SDE::Render;

GDIRender::GDIRender(const MapView* view, HWND hwnd)
	: m_hwnd(hwnd), m_view(view), m_color(0)
{
}

GDIRender::~GDIRender()
{
	DeleteObject(m_memBitmap);
	DeleteDC(m_memDC);

	DeleteObject(m_reserveBitmap);
	DeleteDC(m_reserveDC);
}

void GDIRender::initMemDC(HDC hdc/* = NULL*/)
{
	DeleteObject(m_memBitmap);
	DeleteDC(m_memDC);

	HDC tmpDC = hdc == NULL ? GetDC(m_hwnd) : hdc;

	m_memDC = CreateCompatibleDC(tmpDC);
	m_memBitmap = CreateCompatibleBitmap(tmpDC, m_view->cxClient(), m_view->cyClient());
	SelectObject(m_memDC, m_memBitmap);
	Rectangle(m_memDC, -1, -1, m_view->cxClient() + 2, m_view->cyClient() + 2);

	if (hdc == NULL)
		ReleaseDC(m_hwnd, tmpDC);
}

void GDIRender::initReserveDC(HDC hdc/* = NULL*/)
{
	DeleteObject(m_reserveBitmap);
	DeleteDC(m_reserveDC);

	HDC tmpDC = hdc == NULL ? GetDC(m_hwnd) : hdc;

	m_reserveDC = CreateCompatibleDC(tmpDC);
	m_reserveBitmap = CreateCompatibleBitmap(tmpDC, m_view->cxClient(), m_view->cyClient());
	SelectObject(m_reserveDC, m_reserveBitmap);
	Rectangle(m_reserveDC, -1, -1, m_view->cxClient() + 2, m_view->cyClient() + 2);

	if (hdc == NULL)
		ReleaseDC(m_hwnd, tmpDC);
}

void GDIRender::startMemRender(HDC hdc/* = NULL*/)
{
	if (hdc == NULL)
	{
		HDC tmpDC = GetDC(m_hwnd);
		BitBlt(m_memDC, 0, 0, m_view->cxClient(), m_view->cyClient(), tmpDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hwnd, tmpDC);
	}
	else
		BitBlt(m_memDC, 0, 0, m_view->cxClient(), m_view->cyClient(), hdc, 0, 0, SRCCOPY);
}

void GDIRender::endMemRender(HDC hdc/* = NULL*/)
{
	if (hdc == NULL)
	{
		HDC tmpDC = GetDC(m_hwnd);
		BitBlt(tmpDC, 0, 0, m_view->cxClient(), m_view->cyClient(), m_memDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hwnd, tmpDC);
	}
	else
		BitBlt(hdc, 0, 0, m_view->cxClient(), m_view->cyClient(), m_memDC, 0, 0, SRCCOPY);
}

void GDIRender::storeDC(HDC hdc/* = NULL*/)
{
	if (hdc == NULL)
	{
		HDC tmpDC = GetDC(m_hwnd);
		BitBlt(m_reserveDC, 0, 0, m_view->cxClient(), m_view->cyClient(), tmpDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hwnd, tmpDC);
	}
	else
		BitBlt(m_reserveDC, 0, 0, m_view->cxClient(), m_view->cyClient(), hdc, 0, 0, SRCCOPY);
}

void GDIRender::restoreDC(HDC hdc/* = NULL*/)
{
	if (hdc == NULL)
	{
		HDC tmpDC = GetDC(m_hwnd);
		BitBlt(tmpDC, 0, 0, m_view->cxClient(), m_view->cyClient(), m_reserveDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hwnd, tmpDC);
	}
	else
		BitBlt(hdc, 0, 0, m_view->cxClient(), m_view->cyClient(), m_reserveDC, 0, 0, SRCCOPY);
}

void GDIRender::render(const IShape& s)
{
	const Rect* pr = dynamic_cast<const Rect*>(&s);
	if (pr != 0) {
		render(pr);
		return;
	}

	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != 0) {
		render(ppt);
		return;
	}

	const Line* pl = dynamic_cast<const Line*>(&s);
	if (pl != 0) {
		render(pl);
		return;
	}

	//const Polygon* pp = dynamic_cast<const Polygon*>(&s);
	//if (pp != 0) {
	//	render(pp);
	//	return;
	//}

	throw Tools::IllegalStateException(
		"GDIRender::render: Not implemented yet!"
		);
}

void GDIRender::highlightSelection(const Selection& selection, const Rect& scope)
{
	HGDIOBJ oldSelectObj = SelectObject(m_memDC, CreatePen(PS_SOLID, 2, RGB(0, 0, 255)));

	std::vector<IShape*> shapes;
	selection.getAllShapes(shapes);
	for (size_t i = 0; i < shapes.size(); ++i)
		render(*shapes[i]);
	for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());

	SelectObject(m_memDC, oldSelectObj);
	endMemRender();
}

void GDIRender::render(int x, int y, LPCSTR s, int len)
{
	TextOutA(m_memDC, x, y, s, len);
}

void GDIRender::render(const ScreenScope& scope)
{
	POINT bound[5] = {
		scope.m_xMin, scope.m_yMin, 
		scope.m_xMin, scope.m_yMax, 
		scope.m_xMax, scope.m_yMax, 
		scope.m_xMax, scope.m_yMin, 
		scope.m_xMin, scope.m_yMin
	};

	//	SelectObject(m_memDC, CreatePen(PS_DASH, 0, RGB(255, 0, 0)));
	MoveToEx(m_memDC, bound[0].x, bound[0].y, NULL);
	for (size_t i = 1; i < 5; ++i)
		LineTo(m_memDC, bound[i].x, bound[i].y);
	//	DeleteObject(SelectObject(m_memDC, GetStockObject(BLACK_PEN)));
}

void GDIRender::render(const ScreenPos2D& s, const ScreenPos2D& e)
{
	MoveToEx(m_memDC, s.d0, s.d1, NULL);
	LineTo(m_memDC, e.d0, e.d1);
}

void GDIRender::renderLine(const std::vector<MapPos2D>& line)
{
	size_t len = line.size();
	if (len <= 1)
		return;

	std::vector<ScreenPos2D> screenLine;
	m_view->transMapToScreen(line, &screenLine);

	MoveToEx(m_memDC, screenLine.at(0).d0, screenLine.at(0).d1, NULL);
	for (size_t i = 1; i < len; ++i)
		LineTo(m_memDC, screenLine.at(i).d0, screenLine.at(i).d1);
}

void GDIRender::renderFace(const std::vector<MapPos2D>& face)
{
	assert(face.front() != face.back());

	std::vector<ScreenPos2D> screenFace;
	m_view->transMapToScreen(face, &screenFace);

	size_t len = screenFace.size();
	std::vector<POINT> facePoints;
	for (size_t i = 0; i < len; ++i)
	{
		POINT p;
		p.x = screenFace.at(i).d0;
		p.y = screenFace.at(i).d1;
		facePoints.push_back(p);
	}

	/*HGDIOBJ oldBrush = */SelectObject(m_memDC, GetStockObject(GRAY_BRUSH));
	Polygon(m_memDC, &facePoints.at(0), len);
}

//
// Private
//

void GDIRender::render(const Rect* rect)
{
	long left, right, top, bottom;
	m_view->transMapToScreen(rect->m_pLow[0], rect->m_pLow[1], &left, &bottom);
	m_view->transMapToScreen(rect->m_pHigh[0], rect->m_pHigh[1], &right, &top);
	POINT bound[5] = { left, bottom, left, top, right, top, right, bottom, left, bottom };

	SelectObject(m_memDC, CreatePen(PS_DASH, 0, RGB(255, 0, 0)));
	MoveToEx(m_memDC, bound[0].x, bound[0].y, NULL);
	for (size_t i = 1; i < 5; ++i)
		LineTo(m_memDC, bound[i].x, bound[i].y);
	DeleteObject(SelectObject(m_memDC, GetStockObject(BLACK_PEN)));
}

void GDIRender::render(const Point* point)
{
	long x, y;
	m_view->transMapToScreen(point->m_pCoords[0], point->m_pCoords[1], &x, &y);

	SelectObject(m_memDC, GetStockObject(BLACK_BRUSH));
	Ellipse(m_memDC, x - 2, y + 2, x + 2, y - 2);
}

void GDIRender::render(const Line* line)
{
	size_t len = line->posNum();
	assert(len > 1);
	std::vector<ScreenPos2D> pos;
	pos.reserve(len);

	for (size_t i = 0; i < len; ++i)
	{
		MapPos2D mapPos;
		line->getPoint(i, &mapPos);
		pos.push_back(m_view->transMapToScreen(mapPos));
	}

	MoveToEx(m_memDC, pos.at(0).d0, pos.at(0).d1, NULL);
	for (size_t i = 1; i < len; ++i)
		LineTo(m_memDC, pos.at(i).d0, pos.at(i).d1);
}

//// todo: define other Polygon ?
//void GDIRender::render(const IS_SDE::Polygon* poly)
//{
//	//size_t ringLen = poly->m_ringNum;
//	//for (size_t i = 0; i < ringLen; ++i)
//	//{
//	//	size_t posLen = poly->m_posNum[i];
//	//	for (size_t j = 0; j < posLen; ++j)
//	//	{
//
//	//	}
//	//}
//}
