#include "BindPoint.h"
#include "SingleLayerNW.h"
#include "../../BasicType/Line.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

BindPoint::BindPoint(const MapPos2D& p, const SimulateNavigate::IPointBindLine* bind, const SingleLayerNW* nw)
{
	Line* l;
	bind->bind(p, &l, &m_roadID);

	std::vector<MapPos2D> ctlpos;
	l->getPoint(&ctlpos);

	m_beginPos = ctlpos.front();
	m_endPos = ctlpos.back();

	m_beginID = nw->queryTopoNodeID(m_beginPos);
	m_endID = nw->queryTopoNodeID(m_endPos);

	Tools::SpatialMath::getNearestPosOnLineToPoint(p, ctlpos, &m_pos);
	m_beginLen = Tools::SpatialMath::getDisOnLinePosToLineEnd(m_pos, ctlpos, true);
	m_endLen = Tools::SpatialMath::getDisOnLinePosToLineEnd(m_pos, ctlpos, false);

	delete l;
}

BindPoint::BindPoint(std::ifstream& fin)
{
	fin >> m_pos.d0 >> m_pos.d1 
		>> m_beginPos.d0 >> m_beginPos.d1 >> m_endPos.d0 >> m_endPos.d1 
		>> m_beginID >> m_endID 
		>> m_beginLen >> m_endLen 
		>> m_roadID;
}

void BindPoint::output(std::ofstream& fout) const
{
	fout << std::setprecision(20) 
		<< m_pos.d0 << " " << m_pos.d1 << " " 
		<< m_beginPos.d0 << " " << m_beginPos.d1 << " " << m_endPos.d0 << " " << m_endPos.d1 << " "
		<< m_beginID << " " << m_endID << " " 
		<< m_beginLen << " " << m_endLen << " " 
		<< m_roadID << std::endl;
}