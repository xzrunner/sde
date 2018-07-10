#ifndef _IS_SDE_SHELL_INFO_WINDOW_H_
#define _IS_SDE_SHELL_INFO_WINDOW_H_
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
		const std::string GENERALIZATION_WINDOW_NAME = "MdiGen";

		class FrameWindow;

		class GeneralizationWindow
		{
		public:
			GeneralizationWindow(HINSTANCE hInstance, FrameWindow* pFrameWnd);
			~GeneralizationWindow();

		private:
			void registerClass(HINSTANCE hInstance);

			static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

			static EDIT::EditController* getEditController(HWND hwnd);

		private:
			class EachWndData
			{
			public:
				EachWndData(HWND hwnd, HINSTANCE hInstance);
				~EachWndData();

				void onPaint(HDC hdc);
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
			static GeneralizationWindow* pThis;
			FrameWindow* m_pFrameWnd;

		private:
			HMENU m_hMenuFrameWnd, m_hMenuWnd;

			WndDataManager m_windowsManager;

		}; // GeneralizationWindow
	}
}
#endif // _IS_SDE_SHELL_INFO_WINDOW_H_