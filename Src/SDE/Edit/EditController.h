#ifndef _IS_SDE_EDIT_EDITCONTROLLER_H_
#define _IS_SDE_EDIT_EDITCONTROLLER_H_
#include "../Tools/SpatialDataBase.h"
#include "Utility.h"
#include <windows.h>
#include <basetsd.h>

namespace IS_SDE
{
	namespace Render
	{
		class LayersManager;
		class MapView;
		class MapLayer;
		class GDIRender;
		class Selection;
	}

	namespace EDIT
	{
		class EditTask;
		class EditCommand;

		class EditController
		{
			class WndInfo;

		public:
			EditController(HWND hwnd, HINSTANCE hInstance);
			~EditController();

			void doLButtonDown(long keyflag, long xpos, long ypos);
			void doLButtonUp(long keyflag, long xpos, long ypos);
			void doRButtonDown(long keyflag, long xpos, long ypos);
			void doRButtonUp(long keyflag, long xpos, long ypos);
			void doMouseMove(long keyflag, long xpos, long ypos);
			void doLButtonDblclk(long keyflag, long xpos, long ypos);
			void doMouseWheel(int iDeltaPerLine, int& iAccumDelta, long xpos, long ypos);
			void doCommandLine(const EditCommand& command);

			// change as command type
			void doKeyDown(UINT_PTR wParam);

			void invalidateWnd(bool immediately = false);
			void onPaint(HDC hdc);
			void onPaint(HDC hdc, size_t& objNum, long& fatchTime, long& renderTime);
			void onPaintMem(HDC hdc);

		public:
			Render::MapView* getMapView() const;
			void getVisibleLayers(std::vector<Render::MapLayer*>& layers) const;
			Render::GDIRender* getRender() const { return m_mapRender; }
			WndInfo& getWndInfo() { return m_wndInfo; }
			bool isMapViewEmpty() const;
			Render::Selection* getSelection() { return m_selection; }
			const Render::LayersManager* getLayersMgr() const { return m_layersManager; }
			void getAllShapeData(std::vector<IShape*>& shapes) const;

			// set display or not
			void flipLayerVisible(size_t index);

			// open layer
			bool openLayerToRender(const std::wstring& filePath) const;
			size_t getLayersSize() const;
			bool isLayerExist(const std::wstring& layerName) const;
			bool isNoneVisibleLayer() const;

			// bulk load
			void bukBuildQuadTree(const std::wstring& filePath, size_t attrIndex, long& time);
			void bukBuildRTree(const std::wstring& filePath, long& time);
			void bukBuildNVRTree(const std::wstring& filePath, long& time);

			// Insert 3D point one by one from txt file to a R-Tree
			// Data Format: x0 y0 z0
			//				x1 y1 z1
			//				...
			void insertPoint3DFromTxtFileToANewRTree(const std::wstring& filePath);

			void setUIType(MapEditUIType UIType);
			void createNewTask(MapEditUIType UIType);
			void endTask();

			void changeSetting(SettingUIType UIType);
			bool checkSetting(SettingUIType UIType) const;

			HWND getHWND() const { return m_hwnd; }

		private:
			bool needNewInstanceAfterTaskEnd(MapEditUIType UIType) const;

			std::wstring getIdxFilePath() const;

		private:
			class WndInfo
			{
			public:
				int cxDialogBaseUnits, cyDialogBaseUnits;
			};

			class DisplayInfo
			{
			public:
				bool bDrawGridBorder;
				bool bDrawComputePathProcess;

			public:
				DisplayInfo() : bDrawGridBorder(false), bDrawComputePathProcess(false) {}
			};

		private:
			HWND					m_hwnd;
			HINSTANCE			    m_hInstance;

			EditTask*				m_task;
			Render::LayersManager*	m_layersManager;
			Render::MapView*		m_mapView;

			Render::GDIRender*		m_mapRender;
			Render::Selection*		m_selection;

			WndInfo					m_wndInfo;
			DisplayInfo				m_displayInfo;

			friend class EditTask;

		}; // EditController
	}
}

#endif // _IS_SDE_EDIT_EDITCONTROLLER_H_