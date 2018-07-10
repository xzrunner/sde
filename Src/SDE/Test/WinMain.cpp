//#define _WIN32_WINNT 0x500
//#define UNICODE
//#include <windows.h>
//#include <tchar.h>
//#include "resource.h"
//#include "TestSpatialIndex.h"
//#include "TestFile.h"
//#include "../Tools/StringTools.h"
//
//LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
//{
//	static TCHAR szAppName[] = TEXT("TESTVIEW");
//	HWND		hwnd;
//	MSG			msg;
//	WNDCLASS	wndclass;
//
//	wndclass.style			= CS_HREDRAW | CS_VREDRAW;
//	wndclass.lpfnWndProc	= WndProc;
//	wndclass.cbClsExtra		= 0;
//	wndclass.cbWndExtra		= 0;
//	wndclass.hInstance		= hInstance;
//	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
//	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
//	wndclass.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
//	wndclass.lpszMenuName	= szAppName;
//	wndclass.lpszClassName	= szAppName;
//
//	if (!RegisterClass(&wndclass))
//	{
//		MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
//		return 0;
//	}
//
//	hwnd = CreateWindow(szAppName, TEXT("ZZ test"), WS_OVERLAPPEDWINDOW, 
//		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, szCmdLine);
//
//	ShowWindow(hwnd, iCmdShow);
//	UpdateWindow(hwnd);
//
//	while (GetMessage(&msg, NULL, 0, 0))
//	{
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//	}
//
//	return msg.wParam;
//}
//
//IS_SDE::TEST::LayersManager layersManager;
//IS_SDE::Render::MapView view;
//static OPENFILENAME ofnMif, ofnIdx;
//void outPutPosition(HWND hwnd, double x, double y, long xClient, long yClient);
//void outPutTime(HWND hwnd, long xClient, long yClient, size_t objNum, long tot, long fetch, long render);
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	static int	cxClient, cyClient;
//	static int iDeltaPerLine, iAccumDelta;
//	static bool fBlocking;
//	static POINT ptBeg, ptEnd;
//	ULONG ulScrollLines;
//	HDC			hdc, memDC;
//	PAINTSTRUCT	ps;
//	HBITMAP bitmap;
//	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
//	static int cxChar, cyChar;
//	size_t lType;
//	POINT posMouseWheel;
//	static std::vector<HWND> layerHWnd;
//
//	switch (message)
//	{
//	case WM_CREATE:
//		{
//			IS_SDE::TEST::FileUtility::PopFileInitializeMif(ofnMif, hwnd);
//			IS_SDE::TEST::FileUtility::PopFileInitializeIdx(ofnIdx, hwnd);
//			SendMessage(hwnd, WM_SETTINGCHANGE, 0, 0);
//			cxChar = LOWORD(GetDialogBaseUnits());
//			cyChar = HIWORD(GetDialogBaseUnits());
//		}
//
//		return 0;
//
//	case WM_SIZE:
//		cxClient = LOWORD(lParam);
//		cyClient = HIWORD(lParam);
//		view.setScreenPix(cxClient, cyClient);
//		return 0;
//
//	case WM_LBUTTONDOWN:
//		fBlocking = true;
//		ptBeg.x = ptEnd.x = LOWORD(lParam);
//		ptBeg.y = ptBeg.y = HIWORD(lParam);
//		return 0;
//
//	case WM_LBUTTONUP:
//		if (fBlocking)
//		{
//			fBlocking = false;
//			ptEnd.x = LOWORD(lParam);
//			ptEnd.y = HIWORD(lParam);
//		}
//		return 0;
//
//	case WM_MOUSEMOVE:
//		{
//			double x, y;
//			view.transScreenToMap(LOWORD(lParam), HIWORD(lParam), &x, &y);
//			outPutPosition(hwnd, x, y, 0, cyClient - cyChar);
//
//			if (fBlocking)
//			{
//				ptEnd.x = LOWORD(lParam);
//				ptEnd.y = HIWORD(lParam);
//				view.move(ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y);
//				ptBeg = ptEnd;
//				InvalidateRect(hwnd, NULL, TRUE);
//			}
//		}
//		return 0;
//
//	case WM_SETTINGCHANGE:
//		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);
//		if (ulScrollLines)
//			iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
//		else
//			iDeltaPerLine = 0;
//
//	case WM_KEYDOWN:
//		switch (wParam)
//		{
//		case VK_UP:
//			view.move(0, -1);
//			break;
//		case VK_DOWN:
//			view.move(0, 1);
//			break;
//		case VK_LEFT:
//			view.move(1, 0);
//			break;
//		case VK_RIGHT:
//			view.move(-1, 0);
//			break;
//		}
//		InvalidateRect(hwnd, NULL, TRUE);
//		return 0;
//
//	case WM_MOUSEWHEEL:
//		if (iDeltaPerLine == 0)
//			break;
//		iAccumDelta += (short)HIWORD(wParam);
//
//		GetCursorPos(&posMouseWheel);
//		ScreenToClient(hwnd, &posMouseWheel);
//		while (iAccumDelta >= iDeltaPerLine)
//		{
//			view.zoomIn(posMouseWheel.x, posMouseWheel.y);
//			iAccumDelta -= iDeltaPerLine;
//		}
//
//		while (iAccumDelta <= -iDeltaPerLine)
//		{
//			view.zoomOut(posMouseWheel.x, posMouseWheel.y);
//			iAccumDelta += iDeltaPerLine;
//		}
//
//		InvalidateRect(hwnd, NULL, TRUE);
//		return 0;
//
//	case WM_CHAR:
//		switch (wParam)
//		{
//		case 0x0072: case 0x0052:		// r, R
//			{
//				view.resetViewScope(layersManager.getDisplayScope());
//				InvalidateRect(hwnd, NULL, TRUE);
//			}
//			break;
//		}
//		return 0;
//
//	case WM_PAINT:
//		{
//			clock_t start = clock(); 
//
//			hdc = BeginPaint(hwnd, &ps);
//
//			memDC = CreateCompatibleDC(hdc);
//			bitmap = CreateCompatibleBitmap(hdc, cxClient, cyClient);
//			SelectObject(memDC, bitmap);
//			Rectangle(memDC, -1, -1, cxClient + 2, cyClient + 2);
//
//			size_t objNum = 0;
//			long fatchDataTime = 0, renderTime = 0;
//			IS_SDE::TEST::TestSpatialIndex::displayLayer(layersManager, view, memDC, objNum, fatchDataTime, renderTime);
//
//			BitBlt(hdc, 0, 0, cxClient, cyClient, memDC, 0, 0, SRCCOPY);
//			DeleteObject(bitmap);
//			DeleteDC(memDC);
//
//			EndPaint(hwnd, &ps);
//
//			long totTime = clock() - start;
//			if (!view.isNull())
//				outPutTime(hwnd, cxClient - 60 * cxChar, cyClient - cyChar, objNum, totTime, fatchDataTime, renderTime);
//		}
//		return 0;
//
//	case WM_COMMAND:
//		lType = LOWORD(wParam);
//
//		if (lType < layersManager.m_layers.size())
//		{
//			if (HIWORD(wParam) == 0)
//			{
//				layersManager.setVisible(lType);
//				SendMessage(hwnd, WM_CHAR, 0x0072, 0);	// r
//			}
//			return 0;
//		}
//
//		switch (lType)
//		{
//		case IDM_R_TREE_LOAD:
//			if (IS_SDE::TEST::FileUtility::PopFileOpenDlg(ofnMif, hwnd, szFileName, szTitleName) != 0)
//			{
//				long time;
//				IS_SDE::TEST::TestSpatialIndex::bukBuildRTree(szFileName, time);
//				char positionBuffer[100], title[1] = "";
//				sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
//				MessageBoxA(hwnd, positionBuffer, title, 0);
//			}
//			return 0;
//
//		case IDM_R_TREE_DISPLAY:
//			if (IS_SDE::TEST::FileUtility::PopFileOpenDlg(ofnIdx, hwnd, szFileName, szTitleName) != 0)
//			{
//				size_t layerIndex = layersManager.m_loadTree.size();
//
//				if (layersManager.isLayerExist(szTitleName))
//				{
//					MessageBox(hwnd, TEXT("Layer exist"), TEXT(""), 0);
//					return 0;
//				}
//
//				size_t ret = layersManager.loadRTree(szFileName, szTitleName, view);
//				if (ret == -1)
//				{
//					MessageBox(hwnd, TEXT("file name should end as '_R'"), TEXT(""), 0);
//					return 0;
//				}
//
//				HWND hWndLayer = CreateWindow(TEXT("button"), szTitleName, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
//					0, cyChar * layerIndex, 20 * cxChar, cyChar * 7 / 6,
//					hwnd, (HMENU)layerIndex, NULL, NULL);
//				SendMessage(hWndLayer, BM_SETCHECK, 1, 0);
//				layerHWnd.push_back(hWndLayer);
//
//				SendMessage(hwnd, WM_CHAR, 0x0072, 0);	// r
//			}
//
//			return 0;
//		case IDM_QUAD_TREE_LOAD:
//			if (IS_SDE::TEST::FileUtility::PopFileOpenDlg(ofnMif, hwnd, szFileName, szTitleName) != 0)
//			{
//				long time;
//				IS_SDE::TEST::TestSpatialIndex::bukBuildQuadTree(szFileName, time);
//				char positionBuffer[100], title[1] = "";
//				sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
//				MessageBoxA(hwnd, positionBuffer, title, 0);
//			}
//			return 0;
//
//		case IDM_QUAD_TREE_DISPLAY:
//			if (IS_SDE::TEST::FileUtility::PopFileOpenDlg(ofnIdx, hwnd, szFileName, szTitleName) != 0)
//			{
//				size_t layerIndex = layersManager.m_loadTree.size();
//
//				if (layersManager.isLayerExist(szTitleName))
//				{
//					MessageBox(hwnd, TEXT("Layer exist"), TEXT(""), 0);
//					return 0;
//				}
//
//				size_t ret = layersManager.loadQuadTree(szFileName, szTitleName, view);
//				if (ret == -1)
//				{
//					MessageBox(hwnd, TEXT("file name should end as '_Quad'"), TEXT(""), 0);
//					return 0;
//				}
//
//				HWND hWndLayer = CreateWindow(TEXT("button"), szTitleName, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
//					0, cyChar * layerIndex, 20 * cxChar, cyChar * 7 / 6,
//					hwnd, (HMENU)layerIndex, NULL, NULL);
//				SendMessage(hWndLayer, BM_SETCHECK, 1, 0);
//				layerHWnd.push_back(hWndLayer);
//
//				SendMessage(hwnd, WM_CHAR, 0x0072, 0);	// r
//			}
//
//			return 0;
//
//		case IDM_TEST_STORAGE:
//			if (IS_SDE::TEST::FileUtility::PopFileOpenDlg(ofnIdx, hwnd, szFileName, szTitleName) != 0)
//			{
//				IS_SDE::TEST::SpatialIndexLoader loader(szFileName);
//				IS_SDE::SpatialIndex::CheckStoreStrategy css;
//				loader.getSpatialIndex()->checkStoreState(css);
//
//				std::ofstream fout("c:/test4_25.txt", std::ios_base::app);
//				fout << 
//
//				//HDC hdc = GetDC(hwnd);
//				//char positionBuffer[100];
//				//sprintf(positionBuffer, "index: \t file size = %d \t node num = %d \t page num = %d",
//				//	css.m_indexSize, css.m_indexNum, css.m_indexPageNum);
//				//TextOutA(hdc, 0, cyChar, positionBuffer, strlen(positionBuffer));
//				//sprintf(positionBuffer, "leaf: \t file size = %d \t node num = %d \t page num = %d",
//				//	css.m_leafSize, css.m_leafNum, css.m_leafPageNum);
//				//TextOutA(hdc, 0, cyChar * 3, positionBuffer, strlen(positionBuffer));
//				//ReleaseDC(hwnd, hdc);
//			}
//
//			return 0;
//		}	
//
//		break;
//
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		return 0;
//	}
//	return DefWindowProc(hwnd, message, wParam, lParam);
//}
//
//void outPutPosition(HWND hwnd, double x, double y, long xClient, long yClient)
//{
//	if (x == 0.0 && y == 0.0)
//		return;
//
//	char positionBuffer[100];
//	sprintf(positionBuffer, " x %f   y %f", x, y);
//
//	HDC hdc = GetDC(hwnd);
//	TextOutA(hdc, xClient, yClient, positionBuffer, strlen(positionBuffer));
//	ReleaseDC(hwnd, hdc);
//}
//
//void outPutTime(HWND hwnd, long xClient, long yClient, size_t objNum, long tot, long fetch, long render)
//{
//	char positionBuffer[100];
//	sprintf(positionBuffer, "num: %10.3fk    tot: %d ms    fetch: %d ms    render: %d ms", objNum / 1000.0, tot, fetch, render);
//
//	HDC hdc = GetDC(hwnd);
//	TextOutA(hdc, xClient, yClient, positionBuffer, strlen(positionBuffer));
//	ReleaseDC(hwnd, hdc);
//}