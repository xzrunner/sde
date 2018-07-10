#include "EditTask.h"
#include "EditController.h"
#include "../Render/MapView.h"
#include "../Render/MapLayer.h"
#include "../Render/LayersManager.h"
#include "../Render/Selection.h"
#include "../Render/Render.h"
#include "../BasicType/Point.h"
#include "../SpatialIndex/Utility/Visitor.h"

using namespace IS_SDE;
using namespace IS_SDE::EDIT;

EditTask::EditTask(EditController* editController)
	: m_editController(editController)
{
	setEditMapLayer();
}

void EditTask::doLButtonDown(long keyflag, long xpos, long ypos)
{
	return;
}

void EditTask::doLButtonUp(long keyflag, long xpos, long ypos)
{
	return;
}

void EditTask::doRButtonDown(long keyflag, long xpos, long ypos)
{
	return;
}

void EditTask::doRButtonUp(long keyflag, long xpos, long ypos)
{
	return;
}

void EditTask::doMouseMove(long keyflag, long xpos, long ypos)
{
	EditTask::drawMousePositoin(xpos, ypos);
}

void EditTask::doLButtonDblclk(long keyflag, long xpos, long ypos)
{
	return;
}

void EditTask::doMouseWheel(int iDeltaPerLine, int& iAccumDelta, long xpos, long ypos)
{
	while (iAccumDelta >= iDeltaPerLine)
	{
		m_editController->m_mapView->zoomIn(xpos, ypos);
		iAccumDelta -= iDeltaPerLine;
	}

	while (iAccumDelta <= -iDeltaPerLine)
	{
		m_editController->m_mapView->zoomOut(xpos, ypos);
		iAccumDelta += iDeltaPerLine;
	}
}

void EditTask::doCommandLine(const EditCommand& command)
{
	return;
}

void EditTask::drawMousePositoin(long xpos, long ypos) const
{
	if (m_editController->getMapView()->isNull())
		return;

	double x, y;
	m_editController->getMapView()->transScreenToMap(xpos, ypos, &x, &y);

	char positionBuffer[100];
	sprintf(positionBuffer, " [x]%f   [y]%f", x, y);

	m_editController->getRender()->startMemRender();
	m_editController->getRender()->render(0, m_editController->m_mapView->cyClient() - m_editController->m_wndInfo.cyDialogBaseUnits, positionBuffer, strlen(positionBuffer));
	m_editController->getRender()->endMemRender();
}

void EditTask::execute() 
{ 
	return; 
}

void EditTask::onPaint() 
{ 
	return; 
}

void EditTask::highlightSelection() const
{
	if (!m_editController->m_selection->isEmpty() && !m_editController->m_mapView->isNull())
	{
		m_editController->m_mapRender->highlightSelection(*m_editController->m_selection, m_editController->getMapView()->getViewScope());
	}
}

void EditTask::setEditMapLayer()
{
	m_editLayer = m_editController->m_layersManager->getFirstVisibleMapLayer();
}

void EditTask::transScreenToMap(long s_x, long s_y, double* d_x, double* d_y) const
{
	m_editController->m_mapView->transScreenToMap(s_x, s_y, d_x, d_y);
}

Render::Selection* EditTask::getSelection()
{
	return m_editController->m_selection;
}

void EditTask::endTask() 
{ 
	m_editController->endTask(); 
}

void EditTask::refresh()
{
	m_editController->invalidateWnd();
}

bool EditTask::isPointSelectObjs(long xpos, long ypos)
{
	double coords[2];
	transScreenToMap(xpos, ypos, &coords[0], &coords[1]);
	Point p(coords);

	SpatialIndex::QuerySelectionVisitor vis;

	std::vector<Render::MapLayer*> allLayers;
	m_editController->getVisibleLayers(allLayers);

	size_t i = 0;
	for ( ; i < allLayers.size(); ++i)
	{
		IS_SDE::ISpatialIndex* spatialIndex = allLayers[i]->getSpatialIndex();
		spatialIndex->pointLocationQuery(p, vis, true);
		if (!vis.isEmpty())
		{
			break;
		}
	}

	if (i != allLayers.size())
	{
		m_editController->m_selection->insert(allLayers[i], vis.m_nodeIDWithObjID.front().first, vis.m_nodeIDWithObjID.front().second.front());
		return true;
	}
	else
	{
		return false;
	}
}

void EditTask::pointSelectObjs(long xpos, long ypos)
{
	double coords[2];
	transScreenToMap(xpos, ypos, &coords[0], &coords[1]);
	Point p(coords);

	SpatialIndex::QuerySelectionVisitor vis;

	std::vector<Render::MapLayer*> allLayers;
	m_editController->getVisibleLayers(allLayers);

	for (size_t i = 0; i < allLayers.size(); ++i)
	{
		IS_SDE::ISpatialIndex* spatialIndex = allLayers[i]->getSpatialIndex();
		spatialIndex->pointLocationQuery(p, vis, true);
		m_editController->m_selection->insert(allLayers[i], vis);
		vis.clear();
	}
}

void EditTask::rectSelectObjs(long x0, long y0, long x1, long y1)
{
	double coordsP0[2], coordsP1[2];
	transScreenToMap(x0, y0, &coordsP0[0], &coordsP0[1]);
	transScreenToMap(x1, y1, &coordsP1[0], &coordsP1[1]);

	if (coordsP0[0] > coordsP1[0]) std::swap(coordsP0[0], coordsP1[0]);
	if (coordsP0[1] > coordsP1[1]) std::swap(coordsP0[1], coordsP1[1]);

	Rect rect(coordsP0, coordsP1);

	SpatialIndex::QuerySelectionVisitor vis;

	std::vector<Render::MapLayer*> allLayers;
	m_editController->getVisibleLayers(allLayers);

	m_editController->m_selection->clear();

	for (size_t i = 0; i < allLayers.size(); ++i)
	{
		IS_SDE::ISpatialIndex* spatialIndex = allLayers[i]->getSpatialIndex();
		spatialIndex->intersectsWithQuery(rect, vis, true);
		m_editController->m_selection->insert(allLayers[i], vis);
		vis.clear();
	}
}