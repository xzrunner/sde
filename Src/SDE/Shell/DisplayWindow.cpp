#define _WIN32_WINNT 0x500
#include "DisplayWindow.h"
#include "FrameWindow.h"
#include "../Render/MapView.h"
#include "../Test/TestFile.h"
#include "../Tools/resource.h"
#include "../Edit/EditController.h"
#include "../Tools/StringTools.h"
#include "../Edit/EditCommand.h"

using namespace IS_SDE;
using namespace IS_SDE::SHELL;

DisplayWindow* DisplayWindow::pThis = NULL;

DisplayWindow::DisplayWindow(HINSTANCE hInstance, FrameWindow* pFrameWnd)
	: m_pFrameWnd(pFrameWnd)
{
	pThis = this;
	pThis->m_hMenuFrameWnd = LoadMenu(hInstance, TEXT("MdiMenuDisplay"));
	pThis->m_hMenuWnd = GetSubMenu(pThis->m_hMenuFrameWnd, DISPLAY_MENU_POS);

	registerClass(hInstance);
}

DisplayWindow::~DisplayWindow()
{
	DestroyMenu(pThis->m_hMenuFrameWnd);
}

void DisplayWindow::registerClass(HINSTANCE hInstance)
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
	wndclass.lpszClassName = DISPLAY_WINDOW_NAME.c_str();

	RegisterClass(&wndclass);
}

LRESULT DisplayWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int			cxClient, cyClient;
	static HWND			hwndClient, hwndFrame;
	static int			cxChar, cyChar;				// system's dialog base units
	static OPENFILENAME ofnMif, ofnIdx, ofnTxt;		// open file info
	static int			iDeltaPerLine, iAccumDelta;	// wheel info
	static bool			isTheFirstWnd = true;
	static TCHAR		szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static HINSTANCE	hInstance;
	static size_t		attrSelectItem = -1;

	switch (message)
	{
	case WM_CREATE:
		{
			hInstance  = ((LPCREATESTRUCT)lParam)->hInstance;
			hwndClient = GetParent(hwnd);
			hwndFrame  = GetParent(hwndClient);
			// open file info
			TEST::FileUtility::PopFileInitializeMif(ofnMif, hwnd);
			TEST::FileUtility::PopFileInitializeIdx(ofnIdx, hwnd);
			TEST::FileUtility::PopFileInitializeTxt(ofnTxt, hwnd);
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

			pThis->setTaskMenuState(MF_GRAYED);
			pThis->initSettingMenuState();

			return 0 ;
		}

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
			clock_t start = clock(); 

			HDC			hdc;
			PAINTSTRUCT	ps;
			size_t		objNum = 0;
			long		fatchDataTime = 0, renderTime = 0;

			hdc = BeginPaint(hwnd, &ps);
			pThis->m_windowsManager.findWndData(hwnd)->onPaint(hdc, objNum, fatchDataTime, renderTime);
			EndPaint(hwnd, &ps);

			long totTime = clock() - start;
			if (!pThis->m_windowsManager.findWndData(hwnd)->getEditController()->isMapViewEmpty())
			{
				char positionBuffer[100];
				sprintf(positionBuffer, "num: %10.3fk    tot: %d ms    fetch: %d ms    render: %d ms", objNum / 1000.0, totTime, fatchDataTime, renderTime);
			
				HDC hdc = GetDC(hwnd);
				TextOutA(hdc, cxClient - 60 * cxChar, 0, positionBuffer, strlen(positionBuffer));
				ReleaseDC(hwnd, hdc);
			}

			return 0;
		}

	case WM_KEYDOWN:
		switch (wParam)
		{
			EDIT::EditCommand command;
			
		case VK_ESCAPE:
			command.type = EDIT::ECT_SHORTCUT;
			command.shortcut = EDIT::SK_ESC;
			getEditController(hwnd)->doCommandLine(command);
			break;

		case VK_DELETE:
			command.type = EDIT::ECT_SHORTCUT;
			command.shortcut = EDIT::SK_DELETE;
			getEditController(hwnd)->doCommandLine(command);
			break;

		}
		return 0;

	case WM_CHAR:
		switch (wParam)
		{
			EDIT::EditCommand command;
			
		case 0x0072: case 0x0052:		// r, R
			command.type = EDIT::ECT_SHORTCUT;
			command.shortcut = EDIT::SK_RESET;
			getEditController(hwnd)->doCommandLine(command);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		return 0;

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

					if (editController->isNoneVisibleLayer())
						setTaskMenuState(MF_GRAYED);
					else
						setTaskMenuState(MF_ENABLED);
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

					setTaskMenuState(MF_ENABLED);

					HWND hWndLayer = CreateWindow(TEXT("button"), szTitleName, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
						0, cyChar * layerIndex, 20 * cxChar, cyChar * 7 / 6,
						hwnd, (HMENU)layerIndex, NULL, NULL);
					SendMessage(hWndLayer, BM_SETCHECK, 1, 0);

					InvalidateRect(hwnd, NULL, true);
				}

				return 0;

			case IDM_LOAD_Q_TREE:
				if (TEST::FileUtility::PopFileOpenDlg(ofnMif, hwnd, szFileName, szTitleName) != 0)
				{
					pThis->filePath = Tools::StringTools::stringToWString(szFileName);
					DialogBox(hInstance, TEXT("SELECTATTRBOX"), hwnd, SelectAttrProc);

					long time;

					HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					editController->bukBuildQuadTree(
						Tools::StringTools::stringToWString(szFileName), attrSelectItem, time
						);
					SetCursor(hCursor);

					char positionBuffer[100], title[1] = "";
					sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
					MessageBoxA(hwnd, positionBuffer, title, 0);
				}
				return 0;

			case IDM_LOAD_R_TREE:
				if (TEST::FileUtility::PopFileOpenDlg(ofnMif, hwnd, szFileName, szTitleName) != 0)
				{
					long time;

					HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					editController->bukBuildRTree(
						Tools::StringTools::stringToWString(szFileName), time
						);
					SetCursor(hCursor);

					char positionBuffer[100], title[1] = "";
					sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
					MessageBoxA(hwnd, positionBuffer, title, 0);
				}
				return 0;

			case IDM_LOAD_NVR_TREE:
				if (TEST::FileUtility::PopFileOpenDlg(ofnMif, hwnd, szFileName, szTitleName) != 0)
				{
					long time;

					HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					editController->bukBuildNVRTree(
						Tools::StringTools::stringToWString(szFileName), time
						);
					SetCursor(hCursor);

					char positionBuffer[100], title[1] = "";
					sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
					MessageBoxA(hwnd, positionBuffer, title, 0);
				}
				return 0;

			case IDM_LOAD_POINT3D_TXT:
				if (TEST::FileUtility::PopFileOpenDlg(ofnTxt, hwnd, szFileName, szTitleName) != 0)
				{
					editController->insertPoint3DFromTxtFileToANewRTree(
						Tools::StringTools::stringToWString(szFileName)
						);
				}
				return 0;

				//
				// Edit
				//

			case IDM_DISPLAY_TASK:
				{
					HCURSOR hCursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS)));
					//SetClassLong(editController->getHWND(), GCL_HCURSOR,
					//	(LONG)LoadCursor(NULL, IDC_CROSS));
					editController->createNewTask(EDIT::DISPLAY_VIEW_TASK);
					SetCursor(hCursor);
				}
				return 0;
				
			//case SELECT_ATTR_ITEM:
			//	attrSelectItem = lParam;
			//	return 0;

			case IDM_DRAW_CONTOUR_LINE_TASK:
				editController->createNewTask(EDIT::DRAW_CONTOUR_LINE_TASK);
				return 0;

				//
				// Publish
				//

			case IDM_PUBLISH_NV_DATA:
				editController->createNewTask(EDIT::PUBLISH_NV_DATA_TASK);
				return 0;

				//
				// Setting
				//

			case IDM_SETTING_DRAW_BORDER:
				editController->changeSetting(EDIT::DRAW_BORDER_SETTING);
				if (editController->checkSetting(EDIT::DRAW_BORDER_SETTING))
					CheckMenuItem(pThis->m_hMenuFrameWnd, IDM_SETTING_DRAW_BORDER, MF_CHECKED);
				else
					CheckMenuItem(pThis->m_hMenuFrameWnd, IDM_SETTING_DRAW_BORDER, MF_UNCHECKED);
				return 0;

			case IDM_SETTING_DRAW_PATH_PROCESS:
				editController->changeSetting(EDIT::DRAW_COMPUTE_PATH_PROCESS);
				if (editController->checkSetting(EDIT::DRAW_COMPUTE_PATH_PROCESS))
					CheckMenuItem(pThis->m_hMenuFrameWnd, IDM_SETTING_DRAW_PATH_PROCESS, MF_CHECKED);
				else
					CheckMenuItem(pThis->m_hMenuFrameWnd, IDM_SETTING_DRAW_PATH_PROCESS, MF_UNCHECKED);
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

		DrawMenuBar(hwndFrame);
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

BOOL DisplayWindow::SelectAttrProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hAttrList;
	std::vector<std::string> items;

	switch (message)
	{
	case WM_INITDIALOG:
		hAttrList = GetDlgItem(hwnd, IDC_ATTR_LIST);

		getAllAttrItems(pThis->filePath, &items);

		for (int i = items.size() - 1; i >= 0; --i)
			SendMessage(hAttrList, LB_INSERTSTRING, 0, (LPARAM)items.at(i).c_str());

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			size_t iIndex = SendMessage(GetDlgItem(hwnd, IDC_ATTR_LIST), LB_GETCURSEL, 0, 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, SELECT_ATTR_ITEM, iIndex);

			EndDialog(hwnd, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

EDIT::EditController* DisplayWindow::getEditController(HWND hwnd)
{
	return pThis->m_windowsManager.findWndData(hwnd)->getEditController();
}

void DisplayWindow::setTaskMenuState(int state)
{
	EnableMenuItem(pThis->m_hMenuFrameWnd, IDM_DISPLAY_TASK, state);
	EnableMenuItem(pThis->m_hMenuFrameWnd, IDM_DRAW_CONTOUR_LINE_TASK, state);
	EnableMenuItem(pThis->m_hMenuFrameWnd, IDM_PUBLISH_NV_DATA, state);
}

void DisplayWindow::initSettingMenuState()
{
	CheckMenuItem(pThis->m_hMenuFrameWnd, IDM_SETTING_DRAW_BORDER, MF_UNCHECKED);
}

void DisplayWindow::getAllAttrItems(const std::wstring& mifPath, std::vector<std::string>* ret)
{
	std::ifstream fin(mifPath.c_str());
	if (fin.fail())
		throw Tools::IllegalArgumentException(
		"DisplayWindow::getAllAttrItems: Can't open mif file. "
		);

	std::string sLine;
	while (getline(fin, sLine))
	{	
		if (sLine.empty())
			continue;

		std::string sName, sNum;
		size_t search = sLine.find_first_of(" ");
		if (search != -1)
		{
			sName = sLine.substr(0, search);
			sNum  = sLine.substr(search + 1);

			if (_stricmp(sName.c_str(),"Columns"))
				continue;

			size_t num = atoi(sNum.c_str());
			ret->reserve(num);
			for (size_t i = 0; i < num; ++i)
			{
				getline(fin, sLine);
				if (sLine.empty())
				{
					--i;
					continue;
				}
				ret->push_back(sLine);
			}
		}
	}
}

//
// class DisplayWindow::EachWndData
//

DisplayWindow::EachWndData::EachWndData(HWND hwnd, HINSTANCE hInstance)
	: m_editController(new EDIT::EditController(hwnd, hInstance))
{
}

DisplayWindow::EachWndData::~EachWndData()
{
	delete m_editController;
}

void DisplayWindow::EachWndData::onPaint(HDC hdc, size_t& objNum, long& fatchTime, long& renderTime)
{
	m_editController->onPaint(hdc, objNum, fatchTime, renderTime);
}

//
// class DisplayWindow::WndDataManager
//

DisplayWindow::WndDataManager::WndDataManager()
{
	m_itrLastWnd = m_mapHWND2WndData.end();
}

void DisplayWindow::WndDataManager::insertWndData(HWND hwnd, HINSTANCE hInstance)
{
	std::pair<std::map<HWND, EachWndData*>::iterator, bool> state
		= m_mapHWND2WndData.insert(std::make_pair(hwnd, new DisplayWindow::EachWndData(hwnd, hInstance)));
	m_itrLastWnd = state.first;
}

void DisplayWindow::WndDataManager::deleteWndData(HWND hwnd)
{
	delete findWndData(hwnd);
}

void DisplayWindow::WndDataManager::changeClientExtent(HWND hwnd, int xExtent, int yExtent)
{
	findWndData(hwnd)->getEditController()->getMapView()->setScreenPix(xExtent, yExtent);
}

DisplayWindow::EachWndData* DisplayWindow::WndDataManager::findWndData(HWND hwnd)
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
