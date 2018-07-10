#ifndef _IS_SDE_RENDER_MAP_VIEW_H_
#define _IS_SDE_RENDER_MAP_VIEW_H_
#include "../Tools/Tools.h"
#include "../BasicType/Scope.h"
#include "../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace Render
	{
		class MapView
		{
		public:
			MapView();

			void updateViewScope(const MapScope& scope);
			void resetViewScope(const MapScope& scope);
			
			void setScreenPix(size_t width, size_t height);
			void setViewScope(const MapScope& viewScope);

			Rect getViewScope() const;

			void zoomOut(long x, long y);
			void zoomIn(long x, long y);

			void move(long dx, long dy);
			void move(long b_x, long b_y, long e_x, long e_y);

			bool isNull() const { return m_viewScope.isNull(); }

			int cxClient() const { return static_cast<int>(m_screenPixWidth); }
			int cyClient() const { return static_cast<int>(m_screenPixHeight); }

			//
			//	trans
			//

			void transMapToScreen(const double& s_x, const double& s_y, long* d_x, long* d_y) const;
			void transScreenToMap(long s_x, long s_y, double* d_x, double* d_y) const;

			void transMapToScreen(const MapPos2D& src, ScreenPos2D* dest) const;
			void transScreenToMap(const ScreenPos2D& src, MapPos2D* dest) const;

			ScreenPos2D transMapToScreen(const MapPos2D& src) const;
			MapPos2D transScreenToMap(const ScreenPos2D& src) const;

			void transMapToScreen(const std::vector<MapPos2D>& src, std::vector<ScreenPos2D>* dest) const;
			void transScreenToMap(const std::vector<ScreenPos2D>& src, std::vector<MapPos2D>* dest) const;

		private:
			void setScaleTransform(bool center = false);

		private:
			static const size_t movePixCountEachTime = 10;

			size_t m_screenPixWidth, m_screenPixHeight;

			MapScope m_viewScope;

			double m_pixPerDis, m_disPerPix;

		};	// class MapView
	}
}

#endif // _IS_SDE_RENDER_MAP_VIEW_H_