#ifndef _IS_SDE_RENDER_RENDER_H_
#define _IS_SDE_RENDER_RENDER_H_
#include "MapView.h"
#include "../Tools/SpatialDataBase.h"
#include "../BasicType/Scope.h"
#include "../BasicType/BasicGMType.h"
#include <windows.h>

namespace IS_SDE
{
	class Line;

	namespace Render
	{
		class MapView;
		class Selection;

		class GDIRender/* : public IRander*/
		{
		public:
			GDIRender(const MapView* view, HWND hwnd);
			~GDIRender();

			void initMemDC(HDC hdc = NULL);
			void initReserveDC(HDC hdc = NULL);

			void startMemRender(HDC hdc = NULL);
			void endMemRender(HDC hdc = NULL);

			void storeDC(HDC hdc = NULL);
			void restoreDC(HDC hdc = NULL);

			void render(const IShape& s);

			void highlightSelection(const Selection& selection, const Rect& scope);

			void render(int x, int y, LPCSTR s, int len);
			void render(const ScreenScope& scope);
			void render(const ScreenPos2D& s, const ScreenPos2D& e);
			void renderLine(const std::vector<MapPos2D>& line);
			void renderFace(const std::vector<MapPos2D>& face);

			// shouldn't expose internal member!
			// using to draw different style for user
			HDC getMemDC() const { return m_memDC; }

		private:
			void render(const Rect* rect);
			void render(const Point* point);
			void render(const Line* line);
			//void render(const Polygon* poly);

		private:
			HWND			m_hwnd;
			HDC				m_memDC, m_reserveDC;
			HBITMAP			m_memBitmap, m_reserveBitmap;
			const MapView*	m_view;
			COLORREF		m_color;

		};	// class GDIRender
	}
}

#endif // _IS_SDE_RENDER_RENDER_H_