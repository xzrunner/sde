#ifndef _IS_SDE_SHELL_DISPLAY_WINDOW_H_
#define _IS_SDE_SHELL_DISPLAY_WINDOW_H_	
#include "../Tools/Tools.h"
#include <windows.h>

namespace IS_SDE
{
	namespace EDIT
	{
		class EditController;
	}

	namespace SHELL
	{
		const std::string DISPLAY_WINDOW_NAME = "MdiDisplay";

		class FrameWindow;

		class DisplayWindow
		{
		public:
			DisplayWindow(HINSTANCE hInstance, FrameWindow* pFrameWnd);
			~DisplayWindow();

		private:
			void registerClass(HINSTANCE hInstance);

			static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
			static BOOL CALLBACK SelectAttrProc(HWND, UINT, WPARAM, LPARAM);

			static EDIT::EditController* getEditController(HWND hwnd);

			static void setTaskMenuState(int state);
			static void initSettingMenuState();

			static void getAllAttrItems(const std::wstring& mifPath, std::vector<std::string>* ret);

		private:
			class EachWndData
			{
			public:
				EachWndData(HWND hwnd, HINSTANCE hInstance);
				~EachWndData();

				void onPaint(HDC hdc, size_t& objNum, long& fatchTime, long& renderTime);
				EDIT::EditController* getEditController() { return m_editController; }
				
			private:
				EDIT::EditController* m_editController;

			}; // EachWndData

			class WndDataManager
			{
			public:
				WndDataManager();

				void insertWndData(HWND hwnd, HINSTANCE hInstance);
				void deleteWndData(HWND hwnd);

				void changeClientExtent(HWND hwnd, int xExtent, int yExtent);

				EachWndData* findWndData(HWND hwnd);

			private:
				std::map<HWND, EachWndData*>			m_mapHWND2WndData;
				std::map<HWND, EachWndData*>::iterator	m_itrLastWnd;

			}; // WndDataManager

		private:
			static DisplayWindow* pThis;
			FrameWindow* m_pFrameWnd;

			std::wstring filePath;

			HMENU m_hMenuFrameWnd, m_hMenuWnd;

			WndDataManager m_windowsManager;

		}; // DisplayWindow
	}
}
#endif // _IS_SDE_SHELL_DISPLAY_WINDOW_H_