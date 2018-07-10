#include "SearchingUtility.h"
#include "BindPoint.h"
#include "SingleLayerNW.h"
#include "../../NVDataPublish/SPClassification/Utility.h"
#include "../../Render/Render.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::Network;

//
// class DestInfo
//

DestInfo::DestInfo()
	: m_bGridID(Tools::UINT32_MAX), m_eGridID(Tools::UINT32_MAX), 
	m_bTable(NULL), m_eTable(NULL) 
{
}

DestInfo::~DestInfo()
{
	if (m_bTable == m_eTable)
		delete m_bTable;
	else
	{
		delete m_bTable;
		delete m_eTable;
	}
}

void DestInfo::initByNodePos(const BindPoint& p, const NVDataPublish::ILookUp* table, bool bPreLoadTable)
{
	m_pos = p.m_pos;
	m_beginID = p.m_beginID; m_endID = p.m_endID;
	m_beginLen = p.m_beginLen; m_endLen = p.m_endLen;
	m_roadID = p.m_roadID;

	if (table != NULL)
	{
		m_bGridID = table->getGridID(p.m_beginPos);
		m_eGridID = table->getGridID(p.m_endPos);
		if (bPreLoadTable)
		{
			table->loadTable(m_bGridID, &m_bTable);
			if (m_bGridID == m_eGridID)
				m_eTable = m_bTable;
			else
				table->loadTable(m_eGridID, &m_eTable);
		}
	}
}

void DestInfo::initByNodeID(const SingleLayerNW* nw, size_t nodeID, const NVDataPublish::ILookUp* table, bool bPreLoadTable)
{
	nw->getTopoNodePos(nodeID, &m_pos);
	m_beginID = m_endID = nodeID;
	m_beginLen = m_endLen = 0.0;
	m_roadID = 0;

	if (table != NULL)
	{
		m_bGridID = m_eGridID = table->getGridID(m_pos);
		if (bPreLoadTable)
		{
			table->loadTable(m_bGridID, &m_bTable);
			m_eTable = m_bTable;
		}
	}
}

//
// class VisitedNode
//

VisitedNode::VisitedNode(size_t id, double from, const SingleLayerNW* network, 
						 const NVDataPublish::ILookUp* table, const DestInfo& dest, VisitedNode* prev) 
	: m_nodeID(id), m_highID(0), m_from(from), m_prev(prev)
{
	initToDis(network, table, dest);
}

VisitedNode::VisitedNode(size_t lowID, size_t highID, double from, const SingleLayerNW* network, 
						 const NVDataPublish::ILookUp* table, const DestInfo& dest, VisitedNode* prev)
	: m_nodeID(lowID), m_highID(highID), m_from(from), m_prev(prev)
{
	initToDis(network, table, dest);
}

void VisitedNode::getShortestPath(std::vector<size_t>* pathNodeIDs) const
{
	const VisitedNode* curr = this;
	do 
	{
		pathNodeIDs->push_back(curr->m_nodeID);
		curr = curr->m_prev;
	} while (curr);
}

void VisitedNode::getShortestPath(std::vector<std::pair<NODE_ID, NEAREST_DIS> >* path) const
{
	const VisitedNode* curr = this;
	do 
	{
		path->push_back(std::make_pair(curr->m_nodeID, static_cast<size_t>(curr->m_from)));
		curr = curr->m_prev;
	} while (curr);
}

void VisitedNode::initToDis(const SingleLayerNW* network, const NVDataPublish::ILookUp* table, 
							const DestInfo& dest)
{
	if (dest.beSameNodeID(m_nodeID))
		m_to = dest.getEndingDis(m_nodeID);
	else
	{
		MapPos2D pos;
		network->getTopoNodePos(m_nodeID, &pos);
		double lineDis = dest.getMinDisToHere(pos);

		if (table == NULL || !dest.beHasLookUpTableData())
			m_to = lineDis;
		else
		{
			double tableDis = dest.getMinDisToHere(table, pos);
			if (tableDis < lineDis || tableDis == Tools::DOUBLE_MAX)
				m_to = lineDis;
			else
				m_to = tableDis;
		}
	}
}

//
// class SearchingUtility
//

void SearchingUtility::drawLine(const DisProInfo& info, uint64_t id)
{
	if (info.isValid() && id != 0)
	{
		IS_SDE::Line* line;

		assert(!info.m_layers->empty());
		NVDataPublish::SPC::Utility::getLineByAddress(info.m_layers, id, &line);

		HDC dc = info.m_render->getMemDC();
		SelectObject(dc, CreatePen(PS_SOLID, 2, RGB(0, 0, 255)));
		info.m_render->render(*line);
		DeleteObject(SelectObject(dc, GetStockObject(BLACK_PEN)));
		info.m_render->endMemRender();

		delete line;
	}
}

void SearchingUtility::noSearchPath(const BindPoint& from, const BindPoint& to, 
									std::vector<size_t>* routeNodeID)
{
	if (from.m_roadID == to.m_roadID)
		;
	else if (from.m_beginID == to.m_beginID)
	{
		routeNodeID->push_back(from.m_beginID);
		routeNodeID->push_back(from.m_beginID);
	}
	else if (from.m_beginID == to.m_endID)
	{
		routeNodeID->push_back(from.m_beginID);
		routeNodeID->push_back(from.m_beginID);
	}
	else if (from.m_endID == to.m_beginID)
	{
		routeNodeID->push_back(from.m_endID);
		routeNodeID->push_back(from.m_endID);
	}
	else if (from.m_endID == to.m_endID)
	{
		routeNodeID->push_back(from.m_endID);
		routeNodeID->push_back(from.m_endID);
	}
	else
		assert(0);
}
