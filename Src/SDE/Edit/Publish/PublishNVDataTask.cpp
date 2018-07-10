#include "PublishNVDataTask.h"
#include "../EditController.h"
#include "../../Render/MapLayer.h"
#include "../../Render/MapView.h"
#include "../../Render/Render.h"
#include "../../NVDataPublish/Base.h"
#include "../../NVDataPublish/Publish.h"

using namespace IS_SDE;
using namespace IS_SDE::EDIT;

PublishNVDataTask::PublishNVDataTask(EditController* editController)
	: EditTask(editController), m_precision(0.000005)
{
	m_editController->getRender()->initReserveDC();
	m_editController->getRender()->storeDC();
}

void PublishNVDataTask::doMouseMove(long keyflag, long xpos, long ypos)
{
	drawMousePositoin(xpos, ypos);

	Render::MapView* view = m_editController->getMapView();
	MapPos2D center = view->transScreenToMap(ScreenPos2D(xpos, ypos));
	ScreenPos2D leftLow = view->transMapToScreen(MapPos2D(center.d0 - m_precision, center.d1 - m_precision)),
				rightHigh = view->transMapToScreen(MapPos2D(center.d0 + m_precision, center.d1 + m_precision));
	
	ScreenScope scope;
	scope.m_xMin = leftLow.d0;
	scope.m_yMin = leftLow.d1;
	scope.m_xMax = rightHigh.d0;
	scope.m_yMax = rightHigh.d1;

	m_editController->getRender()->restoreDC();
	m_editController->getRender()->startMemRender();
	m_editController->getRender()->render(scope);
	m_editController->getRender()->endMemRender();
}

void PublishNVDataTask::doLButtonDblclk(long keyflag, long xpos, long ypos)
{
	clock_t start = clock(); 
	execute();
	double time = clock() - start;

	char positionBuffer[100], title[1] = "";
	sprintf(positionBuffer, "used: %10.3fs  ", time / 1000.0);
	MessageBoxA(m_editController->getHWND(), positionBuffer, title, 0);

	endTask();
}

void PublishNVDataTask::onPaint()
{
	m_editController->getRender()->storeDC();
}

void PublishNVDataTask::execute()
{
	std::vector<Render::MapLayer*> mapLayers;
	m_editController->getVisibleLayers(mapLayers);

	Tools::Variant var;
	Tools::PropertySet ps;

	// [Arrangement]
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 1000;
	ps.setProperty("MaxComputeRoadNum", var);

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = m_precision;
	ps.setProperty("Precision", var);

	// [Features]
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 4096;
	ps.setProperty("PageSize", var);

	var.m_varType = Tools::VT_PWCHAR;
	std::wstring path = mapLayers.front()->getFilePath();	
	path = path.substr(0, path.find_last_of('\\'));
	var.m_val.pwcVal = const_cast<wchar_t*>(path.c_str());
	ps.setProperty("FilePath", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 0;
	ps.setProperty("PrecisionTransToInt", var);

	var.m_varType = Tools::VT_ULONG;
	NVDataPublish::CondenseStrategyType cs = NVDataPublish::CondenseStrategyType::CS_NO_OFFSET;
	var.m_val.ulVal = static_cast<size_t>(cs);
	ps.setProperty("CondenseStrategy", var);

	// [Network]
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 50;
	ps.setProperty("Capacity", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = true;
	ps.setProperty("WriteThrough", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 2000;
	ps.setProperty("MaxComputeNodeNum", var);

	// [Multi NW]
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 2000;
	ps.setProperty("MaxNodeNumToCondense", var);

	// [Table]
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 100;
	ps.setProperty("MaxNodeNumPerLookUpTable", var);

	NVDataPublish::Publish::publishNVData(m_editController->getHWND(), mapLayers, ps);
}
