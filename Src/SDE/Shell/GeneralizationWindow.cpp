#define _WIN32_WINNT 0x500
#include "GeneralizationWindow.h"
#include "FrameWindow.h"
#include "../Tools/resource.h"
#include "../Tools/StringTools.h"
#include "../Test/TestFile.h"
#include "../Edit/EditController.h"
#include "../Render/MapView.h"

using namespace IS_SDE;
using namespace IS_SDE::SHELL;

GeneralizationWindow* GeneralizationWindow::pThis = NULL;

GeneralizationWindow::GeneralizationWindow(HINSTANCE hInstance, FrameWindow* pFrameWnd)
	: m_pFrameWnd(pFrameWnd)
{
	pThis = this;
	pThis->m_hMenuFrameWnd = LoadMenu(hInstance, TEXT("MdiMenuInfo"));
	pThis->m_hMenuWnd = GetSubMenu(pThis->m_hMenuFrameWnd, GEN_MENU_POS);

	registerClass(hInstance);
}

GeneralizationWindow::~GeneralizationWindow()
{
	DestroyMenu(pThis->m_hMenuFrameWnd);
}

void GeneralizationWindow::registerClass(HINSTANCE hInstance)
{
	WNDCLASS wndclass;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = GENERALIZATION_WINDOW_NAME.c_str();

	RegisterClass(&wndclass);
}

LRESULT GeneralizationWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int			cxClient, cyClient;
	static HWND			hwndClient, hwndFrame;
	static int			cxChar, cyChar;				// system's dialog base units
	static OPENFILENAME ofnIdx;						// open file info
	static int			iDeltaPerLine, iAccumDelta;	// wheel info
	static bool			isTheFirstWnd = true;
	static TCHAR		szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static HINSTANCE	hInstance;

	switch (message)
	{
	case WM_CREATE:
		hInstance  = ((LPCREATESTRUCT)lParam)->hInstance;
		hwndClient = GetParent(hwnd);
		hwndFrame  = GetParent(hwndClient);
		// open file info
		TEST::FileUtility::PopFileInitializeIdx(ofnIdx, hwnd);
		// get wheel info
		if (isTheFirstWnd)
		{
			SendMessage(hwnd, WM_SETTINGCHANGE, 0, 0);
			isTheFirstWnd = false;
		}
		// dialog base units
		cxChar = LOWORD(GetDialogBaseUnits());
		cyChar = HIWORD(GetDialogBaseUnits());

		pThis->m_windowsManager.insertWndData(hwnd, hInstance);
		getEditController(hwnd)->getWndInfo().cxDialogBaseUnits = cxChar;
		getEditController(hwnd)->getWndInfo().cyDialogBaseUnits = cyChar;

		return 0 ;

	case WM_SIZE:		// If not minimized, save the window size

		if (wParam != SIZE_MINIMIZED)
		{
			cxClient = LOWORD(lParam);
			cyClient = HIWORD(lParam);
			pThis->m_windowsManager.changeClientExtent(hwnd, cxClient, cyClient);
		}

		break;			// WM_SIZE must be processed by DefMDIChildProc

	case WM_SETTINGCHANGE:
		{
			ULONG ulScrollLines;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);
			if (ulScrollLines)
				iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
			else
				iDeltaPerLine = 0;
			return 0;
		}

	case WM_LBUTTONDOWN:
		// todo: key flag
		getEditController(hwnd)->doLButtonDown(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_LBUTTONUP:
		// todo: key flag
		getEditController(hwnd)->doLButtonUp(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_RBUTTONDOWN:
		// todo: key flag
		getEditController(hwnd)->doRButtonDown(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_RBUTTONUP:
		// todo: key flag
		getEditController(hwnd)->doRButtonUp(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		// todo: key flag
		getEditController(hwnd)->doMouseMove(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_LBUTTONDBLCLK:
		// todo: key flag
		getEditController(hwnd)->doLButtonDblclk(0, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		{
			POINT posMouseWheel;
			if (iDeltaPerLine == 0)
				break;
			iAccumDelta += (short)HIWORD(wParam);

			GetCursorPos(&posMouseWheel);
			ScreenToClient(hwnd, &posMouseWheel);
			getEditController(hwnd)->doMouseWheel(iDeltaPerLine, iAccumDelta, posMouseWheel.x, posMouseWheel.y);

			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}

	case WM_PAINT:
		{
			HDC			hdc;
			PAINTSTRUCT	ps;

			hdc = BeginPaint(hwnd, &ps);
			pThis->m_windowsManager.findWndData(hwnd)->onPaint(hdc);
			EndPaint(hwnd, &ps);

			return 0;
		}

	case WM_COMMAND:
		{
			EDIT::EditController* editController = 
				pThis->m_windowsManager.findWndData(hwnd)->getEditController();

			size_t lType = LOWORD(wParam);

			if (lType < editController->getLayersSize())
			{
				if (HIWORD(wParam) == 0)
				{
					editController->flipLayerVisible(lType);
					//SendMessage(hwnd, WM_CHAR, 0x0072, 0);	// r
					InvalidateRect(hwnd, NULL, TRUE);
				}
				return 0;
			}

			switch (lType)
			{
			case IDM_DISPLAY_OPEN:
				if (TEST::FileUtility::PopFileOpenDlg(ofnIdx, hwnd, szFileName, szTitleName) != 0)
				{
					size_t layerIndex = editController->getLayersSize();

					bool bOpen = editController->openLayerToRender(
						Tools::StringTools::stringToWString(szFileName)
						);
					if (!bOpen)
					{
						MessageBox(hwnd, TEXT("Layer exist. "), TEXT(""), 0);
						return 0;
					}

					HWND hWndLayer = CreateWindow(TEXT("button"), szTitleName, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
						0, cyChar * layerIndex, 20 * cxChar, cyChar * 7 / 6,
						hwnd, (HMENU)layerIndex, NULL, NULL);
					SendMessage(hWndLayer, BM_SETCHECK, 1, 0);

					InvalidateRect(hwnd, NULL, true);
				}

				return 0;
			}
		}

	case WM_MDIACTIVATE:
		if (lParam == (LPARAM) hwnd)
			SendMessage(hwndClient, WM_MDISETMENU, (WPARAM)pThis->m_hMenuFrameWnd, 
			(LPARAM)pThis->m_hMenuWnd);
		else
			SendMessage(hwndClient, WM_MDISETMENU, (WPARAM)pThis->m_pFrameWnd->pThis->m_hMenuFrameWnd, 
			(LPARAM)pThis->m_pFrameWnd->pThis->m_hMenuWnd);

		DrawMenuBar(hwndFrame) ;
		return 0 ;

	case WM_QUERYENDSESSION:
	case WM_CLOSE:
		if (IDOK != MessageBox(hwnd, TEXT("OK to close window?"), TEXT("Display"), 
			MB_ICONQUESTION | MB_OKCANCEL))
			return 0;
		break;

	case WM_DESTROY:
		pThis->m_windowsManager.deleteWndData(hwnd);
		return 0;
	}

	return DefMDIChildProc(hwnd, message, wParam, lParam);
}

EDIT::EditController* GeneralizationWindow::getEditController(HWND hwnd)
{
	return pThis->m_windowsManager.findWndData(hwnd)->getEditController();
}

//
// class DisplayWindow::EachWndData
//

GeneralizationWindow::EachWndData::EachWndData(HWND hwnd, HINSTANCE hInstance)
: m_editController(new EDIT::EditController(hwnd, hInstance))
{
}

GeneralizationWindow::EachWndData::~EachWndData()
{
	delete m_editController;
}

void GeneralizationWindow::EachWndData::onPaint(HDC hdc)
{
	m_editController->onPaint(hdc);
}

//
// class DisplayWindow::WndDataManager
//

GeneralizationWindow::WndDataManager::WndDataManager()
{
	m_itrLastWnd = m_mapHWND2WndData.end();
}

void GeneralizationWindow::WndDataManager::insertWndData(HWND hwnd, HINSTANCE hInstance)
{
	std::pair<std::map<HWND, EachWndData*>::iterator, bool> state
		= m_mapHWND2WndData.insert(std::make_pair(hwnd, new GeneralizationWindow::EachWndData(hwnd, hInstance)));
	m_itrLastWnd = state.first;
}

void GeneralizationWindow::WndDataManager::deleteWndData(HWND hwnd)
{
	delete findWndData(hwnd);
}

void GeneralizationWindow::WndDataManager::changeClientExtent(HWND hwnd, int xExtent, int yExtent)
{
	findWndData(hwnd)->getEditController()->getMapView()->setScreenPix(xExtent, yExtent);
}

GeneralizationWindow::EachWndData* GeneralizationWindow::WndDataManager::findWndData(HWND hwnd)
{
	if (m_itrLastWnd != m_mapHWND2WndData.end() && 
		hwnd == m_itrLastWnd->first)
		return m_itrLastWnd->second;

	std::map<HWND, EachWndData*>::iterator itr = m_mapHWND2WndData.find(hwnd);
	if (itr == m_mapHWND2WndData.end())
		throw Tools::IllegalStateException(
		"DisplayWindow::WndProc: not find wnd data data. "
		);

	m_itrLastWnd = itr;
	return itr->second;
}
