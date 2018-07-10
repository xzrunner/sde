#include "FrameWindow.h"
#include "DisplayWindow.h"
#include "GeneralizationWindow.h"

using namespace IS_SDE::SHELL;

FrameWindow* frameWnd;
DisplayWindow* displayWnd;
GeneralizationWindow* infoWnd;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HWND     hwndFrame, hwndClient ;
	MSG      msg;

	// Register the frame window class
	frameWnd = new FrameWindow(hInstance);

	// Register the Hello child window class
	displayWnd = new DisplayWindow(hInstance, frameWnd);

	// Register the Rect child window class
	infoWnd = new GeneralizationWindow(hInstance, frameWnd);

	// Create the frame window
	hwndFrame = frameWnd->create();
	hwndClient = GetWindow(hwndFrame, GW_CHILD) ;

	ShowWindow (hwndFrame, iCmdShow) ;
	UpdateWindow (hwndFrame) ;

	// Enter the modified message loop

	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (!TranslateMDISysAccel (hwndClient, &msg))
		{
			TranslateMessage (&msg) ;
			DispatchMessage (&msg) ;
		}
	}
	// Clean up by deleting unattached menus

	delete frameWnd;
	delete displayWnd;
	delete infoWnd;

	return msg.wParam ;
}
