#ifndef _IS_SDE_SHELL_FRAME_WINDOW_H_
#define _IS_SDE_SHELL_FRAME_WINDOW_H_
#include "../Tools/Tools.h"
#include <windows.h>

namespace IS_SDE
{
	namespace SHELL
	{
		#define INIT_MENU_POS    0
		#define DISPLAY_MENU_POS 5
		#define GEN_MENU_POS     4

		#define IDM_FIRSTCHILD   50000

		const std::string FRAME_WINDOW_NAME = "MdiFrame";

		class FrameWindow
		{
		public:
			FrameWindow(HINSTANCE hInstance);
			~FrameWindow();
			HWND create();

		private:
			void registerClass();

		private:
			static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
			static BOOL CALLBACK	CloseEnumProc(HWND, LPARAM);

		private:
			static FrameWindow* pThis;

		private:
			HMENU m_hMenuFrameWnd, m_hMenuWnd;
			HINSTANCE m_hInst;
			
			friend class DisplayWindow;
			friend class GeneralizationWindow;
		}; // FrameWindow
	}
}
#endif // _IS_SDE_SHELL_FRAME_WINDOW_H_