 
#include "../SpatialIndex/Utility/Visitor.h"

#include "EditController.h"
#include "EditTask.h"
#include "DisplayViewTask.h"
#include "DrawContourLineTask.h"
#include "Publish/PublishNVDataTask.h"

#include "EditCommand.h"
#include "../Render/LayersManager.h"
#include "../Render/MapView.h"
#include "../Render/Render.h"
#include "../Render/Selection.h"
#include "DisplayViewTask.h"
#include "../Render/MapLayer.h"
#include "../Adapter/MIFStream.h"
#include "../Adapter/TXTStream.h"
#include "../SpatialIndex/RTree/Utility.h"
#include "../SpatialIndex/NVRTree/Utility.h"
#include "../SpatialIndex/QuadTree/Utility.h"

#include "../NVDataPublish/Network/Publish.h"

#include "../Test/TestFile.h"
#include "../Tools/StringTools.h"

using namespace IS_SDE;
using namespace IS_SDE::EDIT;

// m_task always set as DisplayViewTask, until choose other task
EditController::EditController(HWND hwnd, HINSTANCE hInstance)
	: 
	m_hwnd(hwnd),
	m_hInstance(hInstance),
	m_layersManager(new Render::LayersManager), 
	m_mapView(new Render::MapView),
	m_mapRender(new Render::GDIRender(m_mapView, hwnd)),
	m_selection(new Render::Selection)
{
	m_task = new DisplayViewTask(this);
}

EditController::~EditController()
{
	if (!m_mapView->isNull())
		m_layersManager->storeDisplayRegion(m_mapView->getViewScope());

	delete m_task;
	delete m_selection;
	delete m_mapRender;
	delete m_mapView;
	delete m_layersManager;
}

void EditController::doLButtonDown(long keyflag, long xpos, long ypos)
{
	m_task->doLButtonDown(keyflag, xpos, ypos);
}

void EditController::doLButtonUp(long keyflag, long xpos, long ypos)
{
	m_task->doLButtonUp(keyflag, xpos, ypos);
}

void EditController::doRButtonDown(long keyflag, long xpos, long ypos)
{
	m_task->doRButtonDown(keyflag, xpos, ypos);
}

void EditController::doRButtonUp(long keyflag, long xpos, long ypos)
{
	m_task->doRButtonUp(keyflag, xpos, ypos);
}

void EditController::doMouseMove(long keyflag, long xpos, long ypos)
{
	m_task->doMouseMove(keyflag, xpos, ypos);
}

void EditController::doLButtonDblclk(long keyflag, long xpos, long ypos)
{
	m_task->doLButtonDblclk(keyflag, xpos, ypos);
}

void EditController::doMouseWheel(int iDeltaPerLine, int& iAccumDelta, long xpos, long ypos)
{
	m_task->doMouseWheel(iDeltaPerLine, iAccumDelta, xpos, ypos);
}

void EditController::doCommandLine(const EditCommand& command)
{
	if (command.type == ECT_SHORTCUT)
	{
		switch (command.shortcut)
		{
		case SK_RESET:
			{
				Rect* r(m_layersManager->getTotDisplayScope());
				m_mapView->resetViewScope(MapScope(*r));
				delete r;
			}
			break;
		}
	}

	m_task->doCommandLine(command);
}

void EditController::invalidateWnd(bool immediately/* = false*/)
{
	InvalidateRect(m_hwnd, NULL, true);
	if (immediately)
		UpdateWindow(m_hwnd);
}

void EditController::onPaint(HDC hdc)
{
	if (m_mapView->isNull())
		return;

	std::vector<IShape*> shapes, bounds;
	SpatialIndex::FetchDataAndGridMBRVisitor vis(shapes, bounds);
	Rect r(m_mapView->getViewScope());
	std::vector<Render::MapLayer*> mapLayers;
	getVisibleLayers(mapLayers);
	for (size_t i = 0; i < mapLayers.size(); ++i)
		mapLayers[i]->getSpatialIndex()->intersectsWithQuery(r, vis, false);

	m_mapRender->initMemDC(hdc);
	m_mapRender->startMemRender(hdc);
	for (size_t i = 0; i < shapes.size(); ++i)
		m_mapRender->render(*shapes[i]);
	if (checkSetting(DRAW_BORDER_SETTING))
	{
		for (size_t i = 0; i < bounds.size(); ++i)
			m_mapRender->render(*bounds[i]);
	}
	m_mapRender->endMemRender();

	if (m_task)
	{
		m_task->onPaint();
		m_task->highlightSelection();
	}

	for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
	for_each(bounds.begin(), bounds.end(), Tools::DeletePointerFunctor<IShape>());
}

void EditController::onPaint(HDC hdc, size_t& objNum, long& fatchTime, long& renderTime)
{
	if (m_mapView->isNull())
		return;

	clock_t start = clock();

	std::vector<IShape*> shapes, bounds;
	SpatialIndex::FetchDataAndGridMBRVisitor vis(shapes, bounds);
	Rect r(m_mapView->getViewScope());
	std::vector<Render::MapLayer*> mapLayers;
	getVisibleLayers(mapLayers);
	for (size_t i = 0; i < mapLayers.size(); ++i)
		mapLayers[i]->getSpatialIndex()->intersectsWithQuery(r, vis, false);

	fatchTime = clock() - start;
	start = clock();

	m_mapRender->initMemDC(hdc);
	m_mapRender->startMemRender(hdc);
	for (size_t i = 0; i < shapes.size(); ++i)
		m_mapRender->render(*shapes[i]);
	objNum = shapes.size();
	if (checkSetting(DRAW_BORDER_SETTING))
	{
		for (size_t i = 0; i < bounds.size(); ++i)
			m_mapRender->render(*bounds[i]);
	}
	m_mapRender->endMemRender();

	if (m_task)
	{
		m_task->onPaint();
		m_task->highlightSelection();
	}

	renderTime = clock() - start;

	for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
	for_each(bounds.begin(), bounds.end(), Tools::DeletePointerFunctor<IShape>());
}

Render::MapView* EditController::getMapView() const
{
	return m_mapView;
}

void EditController::getVisibleLayers(std::vector<Render::MapLayer*>& layers) const
{
	m_layersManager->getVisibleLayers(layers);
}

bool EditController::isMapViewEmpty() const
{
	return m_mapView->isNull();
}

void EditController::getAllShapeData(std::vector<IShape*>& shapes) const
{
	SpatialIndex::FetchDataVisitor vis(shapes);
	Rect* r(m_layersManager->getTotDisplayScope());
	std::vector<Render::MapLayer*> mapLayers;
	getVisibleLayers(mapLayers);
	for (size_t i = 0; i < mapLayers.size(); ++i)
		mapLayers[i]->getSpatialIndex()->intersectsWithQuery(*r, vis, false);
	delete r;
}

void EditController::flipLayerVisible( size_t index )
{
	m_layersManager->flipVisible(index);
	m_task->setEditMapLayer();
}

bool EditController::openLayerToRender(const std::wstring& filePath) const
{
	return m_layersManager->loadLayer(filePath, *m_mapView);
}

size_t EditController::getLayersSize() const
{
	return m_layersManager->layersSize();
}

bool EditController::isLayerExist(const std::wstring& layerName) const
{
	return m_layersManager->isLayerExist(layerName);
}

bool EditController::isNoneVisibleLayer() const
{
	return m_layersManager->isNoneVisible();
}

void EditController::bukBuildQuadTree(const std::wstring& filePath, size_t attrIndex, long& time)
{
	std::wstring idxPath = filePath.substr(0, filePath.find_last_of('.')) + L"_Quad";
	IStorageManager* file = StorageManager::createNewDiskStorageManager(idxPath, 4096);
	StorageManager::IBuffer* buffer = StorageManager::createNewRandomEvictionsBuffer(*file, 10, false);

//	std::vector<size_t> attrIndex;
//	attrIndex.push_back(attrIndex);
	Adapter::MIFStream stream(filePath/*, attrIndex*/);

	id_type indexIdentifier;

	clock_t start = clock(); 

	ISpatialIndex* tree = createAndBulkLoadNewQuadTree(
		SpatialIndex::QuadTree::BLM_TYPE0, stream, *buffer, 1, 4, indexIdentifier
		// fillFactor = 0.7
		);

	time = clock() - start;

	delete tree;
	delete buffer;
	delete file;
}

void EditController::bukBuildRTree(const std::wstring& filePath, long& time)
{
	std::wstring idxPath = filePath.substr(0, filePath.find_last_of('.')) + L"_R";
	IStorageManager* file = StorageManager::createNewDiskStorageManager(idxPath, 4096);
	StorageManager::IBuffer* buffer = StorageManager::createNewRandomEvictionsBuffer(*file, 10, false);

	Adapter::MIFStream stream(filePath);

	id_type indexIdentifier;

	clock_t start = clock(); 

	ISpatialIndex* tree = createAndBulkLoadNewRTree(
		SpatialIndex::RTree::BLM_STR, stream, *buffer, 0.7, 100, 100, SpatialIndex::RTree::RV_RSTAR, indexIdentifier
		);

	time = clock() - start;

	delete tree;
	delete buffer;
	delete file;
}

void EditController::bukBuildNVRTree(const std::wstring& filePath, long& time)
{
	std::wstring idxPath = filePath.substr(0, filePath.find_last_of('.')) + L"_NVR";
	IStorageManager* file = StorageManager::createNewDiskStorageManager(idxPath, 4096);
	StorageManager::IBuffer* buffer = StorageManager::createNewRandomEvictionsBuffer(*file, 10, false);

	Adapter::MIFStream stream(filePath);

	id_type indexIdentifier;

	clock_t start = clock(); 

	// fillFactor = 0.7, size = 100
	ISpatialIndex* tree = createAndBulkLoadNewNVRTree(
		SpatialIndex::NVRTree::BLM_STR, stream, *buffer, 0.7, 100, 100, indexIdentifier
		);

	time = clock() - start;

	delete tree;
	delete buffer;
	delete file;
}

void EditController::insertPoint3DFromTxtFileToANewRTree(const std::wstring& filePath)
{
	std::wstring idxPath = filePath.substr(0, filePath.find_last_of('.')) + L"_R";
	IStorageManager* file = StorageManager::createNewDiskStorageManager(idxPath, 4096);
	StorageManager::IBuffer* buffer = StorageManager::createNewRandomEvictionsBuffer(*file, 10, false);

	Adapter::TXTStream stream(filePath);

	id_type indexIdentifier;

	ISpatialIndex* tree = createAndBulkLoadNewRTree(
		SpatialIndex::RTree::BLM_STR, stream, *buffer, 0.7, 100, 100, SpatialIndex::RTree::RV_RSTAR, indexIdentifier
		);

	delete tree;
	delete buffer;
	delete file;
}

void EditController::setUIType(MapEditUIType UIType)
{
	createNewTask(UIType);
}

void EditController::createNewTask(MapEditUIType UIType)
{
	delete m_task, m_task = NULL;

	switch (UIType)
	{
	case DISPLAY_VIEW_TASK:
		m_task = new DisplayViewTask(this);
		break;
	case DRAW_CONTOUR_LINE_TASK:
		// todo
		//m_task = new DrawContourLineTask(this);
		//m_task->execute();
		break;
	case PUBLISH_NV_DATA_TASK:
		m_task = new PublishNVDataTask(this);
		break;
	default:
		throw Tools::IllegalArgumentException(
			"EditController::createNewTask: error type. "
			);
	}
}

void EditController::endTask()
{
	MapEditUIType type = DISPLAY_VIEW_TASK;
	if (m_task)
	{
		invalidateWnd();
		if (needNewInstanceAfterTaskEnd(m_task->taskType()))
			type = m_task->taskType();
	}
	createNewTask(type);
}

void EditController::changeSetting(SettingUIType UIType)
{
	switch (UIType)
	{
	case DRAW_BORDER_SETTING:
		m_displayInfo.bDrawGridBorder = !m_displayInfo.bDrawGridBorder;
		break;
	case DRAW_COMPUTE_PATH_PROCESS:
		m_displayInfo.bDrawComputePathProcess = !m_displayInfo.bDrawComputePathProcess;
		break;
	default:
		throw Tools::IllegalArgumentException(
			"EditController::changeSetting: error type. "
			);
	}

	InvalidateRect(m_hwnd, NULL, TRUE);
}

bool EditController::checkSetting(SettingUIType UIType) const
{
	switch (UIType)
	{
	case DRAW_BORDER_SETTING:
		return m_displayInfo.bDrawGridBorder;
	case DRAW_COMPUTE_PATH_PROCESS:
		return m_displayInfo.bDrawComputePathProcess;
	default:
		throw Tools::IllegalArgumentException(
			"EditController::checkSetting: error type. "
			);
	}
}

bool EditController::needNewInstanceAfterTaskEnd(MapEditUIType UIType) const
{
	switch (UIType)
	{
	case DISPLAY_VIEW_TASK:
		return true;
	case DRAW_CONTOUR_LINE_TASK:
	case PUBLISH_NV_DATA_TASK:
		return false;
	default:
		throw Tools::IllegalArgumentException(
			"EditController::needNewInstanceAfterTaskEnd: error type. "
			);
	}
}

std::wstring EditController::getIdxFilePath() const
{
	static OPENFILENAME ofnIdx;
	TEST::FileUtility::PopFileInitializeIdx(ofnIdx, m_hwnd);
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
	if (TEST::FileUtility::PopFileOpenDlg(ofnIdx, m_hwnd, szFileName, szTitleName) != 0)
	{
		std::wstring path = Tools::StringTools::stringToWString(szFileName);
		return path.substr(0, path.find_last_of('.'));
	}
	else
		throw Tools::IllegalStateException("EditController::getIdxFilePath: error.");
}