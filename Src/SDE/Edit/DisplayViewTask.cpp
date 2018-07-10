#include "DisplayViewTask.h"
#include "../BasicType/Scope.h"
#include "EditController.h"
#include "../Render/Render.h"

using namespace IS_SDE::EDIT;

DisplayViewTask::DisplayViewTask(EditController* editController)
	: EditTask(editController), m_bFinishFirstPoint(false), m_firstX(0), m_firstY(0)
{
}

void DisplayViewTask::doLButtonDown(long keyflag, long xpos, long ypos)
{
	if (!m_bFinishFirstPoint && isPointSelectObjs(xpos, ypos))
	{
		endTask();
		return;
	}

	if (m_bFinishFirstPoint)
	{
		rectSelectObjs(m_firstX, m_firstY, xpos, ypos);
		m_bFinishFirstPoint = false;
		endTask();
	}
	else
	{
		m_firstX = xpos;
		m_firstY = ypos;
		m_bFinishFirstPoint = true;

		m_editController->getRender()->initReserveDC();
		m_editController->getRender()->storeDC();
	}
}

void DisplayViewTask::doMouseMove(long keyflag, long xpos, long ypos)
{
	if (!m_bFinishFirstPoint)
	{
		drawMousePositoin(xpos, ypos);
		m_editController->getRender()->endMemRender();
		return;
	}

	ScreenScope scope(ScreenPos2D(xpos, ypos), ScreenPos2D(m_firstX, m_firstY));
	m_editController->getRender()->restoreDC();
	drawMousePositoin(xpos, ypos);
	m_editController->getRender()->render(scope);
	m_editController->getRender()->endMemRender();
}

void DisplayViewTask::onPaint()
{
	m_editController->getRender()->storeDC();
}