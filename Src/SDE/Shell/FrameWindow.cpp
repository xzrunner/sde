#include "FrameWindow.h"
#include "../Tools/resource.h"
#include "DisplayWindow.h"
#include "GeneralizationWindow.h"

using namespace IS_SDE::SHELL;

FrameWindow* FrameWindow::pThis = NULL;

FrameWindow::FrameWindow(HINSTANCE hInstance)
{
	pThis = this;
	pThis->m_hInst = hInstance;
	pThis->m_hMenuFrameWnd = LoadMenu(hInstance, TEXT("MdiMenuInit"));
	pThis->m_hMenuWnd = GetSubMenu(pThis->m_hMenuFrameWnd, INIT_MENU_POS);

	registerClass();
}

FrameWindow::~FrameWindow()
{
	DestroyMenu(m_hMenuFrameWnd);
}

HWND FrameWindow::create()
{
	return 	CreateWindow (FRAME_WINDOW_NAME.c_str(), TEXT ("MDI Demonstration"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, pThis->m_hMenuFrameWnd, pThis->m_hInst, NULL) ;
}

void FrameWindow::registerClass()
{
	WNDCLASS wndclass;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = pThis->m_hInst;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = FRAME_WINDOW_NAME.c_str();

	if (!RegisterClass (&wndclass))
	{
		MessageBox (NULL, TEXT ("This program requires Windows NT!"),
			FRAME_WINDOW_NAME.c_str(), MB_ICONERROR) ;
	}
}

LRESULT CALLBACK FrameWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND			hwndClient;
	CLIENTCREATESTRUCT	clientcreate;
	HWND				hwndChild;
	MDICREATESTRUCT		mdicreate;

	switch (message)
	{
	case WM_CREATE:

		clientcreate.hWindowMenu	= FrameWindow::pThis->m_hMenuWnd;
		clientcreate.idFirstChild	= IDM_FIRSTCHILD;
		hwndClient = CreateWindow(TEXT ("MDICLIENT"), NULL,
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,   
			0, 0, 0, 0, hwnd, (HMENU) 1, FrameWindow::pThis->m_hInst,
			(PSTR)&clientcreate);
		return 0;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDM_FILE_NEW_DISPLAY:		// Create a Display child window

			mdicreate.szClass = DISPLAY_WINDOW_NAME.c_str();
			mdicreate.szTitle = TEXT("Display");
			mdicreate.hOwner  = FrameWindow::pThis->m_hInst;
			mdicreate.x       = CW_USEDEFAULT;
			mdicreate.y       = CW_USEDEFAULT;
			mdicreate.cx      = CW_USEDEFAULT;
			mdicreate.cy      = CW_USEDEFAULT;
			mdicreate.style   = 0;
			mdicreate.lParam  = 0;

			hwndChild = (HWND)SendMessage(hwndClient, WM_MDICREATE, 0,
				(LPARAM)(LPMDICREATESTRUCT)&mdicreate);
			return 0;

		case IDM_FILE_NEW_GENERALIZATION:		// Create a Info child window

			mdicreate.szClass = GENERALIZATION_WINDOW_NAME.c_str();
			mdicreate.szTitle = TEXT("Generalization");
			mdicreate.hOwner  = pThis->m_hInst;
			mdicreate.x       = CW_USEDEFAULT;
			mdicreate.y       = CW_USEDEFAULT;
			mdicreate.cx      = CW_USEDEFAULT;
			mdicreate.cy      = CW_USEDEFAULT;
			mdicreate.style   = 0;
			mdicreate.lParam  = 0;

			hwndChild = (HWND)SendMessage(hwndClient, WM_MDICREATE, 0,
				(LPARAM)(LPMDICREATESTRUCT)&mdicreate);
			return 0;

		case IDM_FILE_CLOSE:			// Close the active window

			hwndChild = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0);

			if (SendMessage(hwndChild, WM_QUERYENDSESSION, 0, 0))
				SendMessage(hwndClient, WM_MDIDESTROY, (WPARAM)hwndChild, 0);
			return 0 ;

		case IDM_APP_EXIT:				// Exit the program
			SendMessage (hwnd, WM_CLOSE, 0, 0) ;
			return 0 ;

			// messages for arranging windows

		case IDM_WINDOW_TILE:
			SendMessage(hwndClient, WM_MDITILE, 0, 0);
			return 0 ;

		case IDM_WINDOW_CASCADE:
			SendMessage(hwndClient, WM_MDICASCADE, 0, 0);
			return 0 ;

		case IDM_WINDOW_ARRANGE:
			SendMessage(hwndClient, WM_MDIICONARRANGE, 0, 0);
			return 0 ;

		case IDM_WINDOW_CLOSEALL:     // Attempt to close all children
			EnumChildWindows (hwndClient, CloseEnumProc, 0) ;
			return 0 ;

		default:            // Pass to active child...

			hwndChild = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0);

			if (IsWindow(hwndChild))
				SendMessage(hwndChild, WM_COMMAND, wParam, lParam);

			break;			// ...and then to DefFrameProc
		}
		break ;

	case WM_QUERYENDSESSION:
	case WM_CLOSE:			// Attempt to close all children

		SendMessage (hwnd, WM_COMMAND, IDM_WINDOW_CLOSEALL, 0) ;

		if (NULL != GetWindow (hwndClient, GW_CHILD))
			return 0 ;

		break ;				// i.e., call DefFrameProc 

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0 ;
	}

	return DefFrameProc(hwnd, hwndClient, message, wParam, lParam);
}

BOOL FrameWindow::CloseEnumProc(HWND hwnd, LPARAM lParam)
{
	//if (GetWindow (hwnd, GW_OWNER))         // Check for icon title
	//	return TRUE ;

	//SendMessage (GetParent (hwnd), WM_MDIRESTORE, (WPARAM) hwnd, 0) ;

	if (!SendMessage (hwnd, WM_QUERYENDSESSION, 0, 0))
		return TRUE ;

	SendMessage (GetParent (hwnd), WM_MDIDESTROY, (WPARAM) hwnd, 0) ;
	return TRUE ;
}
