#include "Visitor.h"
#include "../../BasicType/Line.h"
#include "../../NVDataPublish/Arrangement/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex;

//
//	FetchDataVisitor
//

void FetchDataVisitor::visitData(const IData& d, size_t dataIndex)
{
	IShape* pS;
	d.getShape(&pS);
	m_shapes.push_back(pS);
}

void FetchDataVisitor::visitData(std::vector<const IData*>& v)
{
	throw Tools::NotSupportedException("FetchDataVisitor::visitData: Should never be called. ");
}

//
//	FetchDataAndGridMBRVisitor
//

void FetchDataAndGridMBRVisitor::visitNode(const INode& n)
{
	if (n.isLeaf())
	{
		IShape* s;
		n.getShape(&s);
		Rect* r = dynamic_cast<Rect*>(s);
		assert(r);
		m_bounds.push_back(r);
	}
}

void FetchDataAndGridMBRVisitor::visitData(const IData& d, size_t dataIndex)
{
	IShape* pS;
	d.getShape(&pS);
	m_shapes.push_back(pS);
}

void FetchDataAndGridMBRVisitor::visitData(std::vector<const IData*>& v)
{
	throw Tools::NotSupportedException("FetchDataAndGridMBRVisitor::visitData: Should never be called. ");
}

//
//	QuerySelectionVisitor
//

void QuerySelectionVisitor::visitNode(const INode& n)
{
	if (n.isLeaf())
	{
		m_nodeID = n.getIdentifier();
	}
}

void QuerySelectionVisitor::visitData(const IData& d, size_t dataIndex)
{
	if (m_nodeIDWithObjID.empty() || m_nodeIDWithObjID.back().first != m_nodeID)
	{
		std::vector<id_type> ids;
		m_nodeIDWithObjID.push_back(std::make_pair(m_nodeID, ids));
	}

	m_nodeIDWithObjID.back().second.push_back(d.getIdentifier());
}

void QuerySelectionVisitor::visitData(std::vector<const IData*>& v)
{
	throw Tools::NotSupportedException("FetchDataVisitor::visitData: Should never be called. ");
}

void QuerySelectionVisitor::clear()
{
	m_nodeIDWithObjID.clear();
}

bool QuerySelectionVisitor::isEmpty() const
{
	return m_nodeIDWithObjID.empty();
}

//
// GetGeoLineVisitor
//

GetGeoLineVisitor::GetGeoLineVisitor(const double& precision, ISpatialIndex* index, 
									 size_t layerID, NVDataPublish::Arrangement::LineMgr* lineMgr)
	: m_precision(precision), m_index(index), m_layerID(layerID), m_lineMgr(lineMgr)
{
}

void GetGeoLineVisitor::visitNode(const INode& n)
{
	m_nodeID = n.getIdentifier();
}

void GetGeoLineVisitor::visitData(const IData& d, size_t dataIndex)
{
	id_type objID = d.getIdentifier();
	std::pair<std::set<id_type>::iterator, bool> state = m_setIDs.insert(objID);
	if (!state.second)
		return;

	IShape* pS;
	d.getShape(&pS);

	Line* l = dynamic_cast<Line*>(pS);
	if (l != NULL)
	{
		m_lineMgr->insertGeoLine(new GeoLine(m_index, l, m_layerID, m_nodeID, objID, dataIndex), m_precision);
	}
	else
	{
		delete pS;
	}
}

void GetGeoLineVisitor::visitData(std::vector<const IData*>& v)
{
	throw Tools::NotSupportedException("GetGeoLineVisitor::visitData: Should never be called. ");
}

//
// GetSelectedGridsMBRVisitor
//

GetSelectedGridsMBRVisitor::GetSelectedGridsMBRVisitor(Rect* ret)
	: m_mbr(ret)
{
	m_mbr->makeInfinite();
}

void GetSelectedGridsMBRVisitor::visitNode(const INode& n)
{
	IShape* shape;
	n.getShape(&shape);
	Rect* mbr = dynamic_cast<Rect*>(shape);
	assert(mbr);

	m_mbr->combineRect(*mbr);

	delete shape;
}

void GetSelectedGridsMBRVisitor::visitData(const IData& d, size_t dataIndex)
{
}

void GetSelectedGridsMBRVisitor::visitData(std::vector<const IData*>& v)
{
	throw Tools::NotSupportedException("GetSelectedGridsMBRVisitor::visitData: Should never be called. ");
}
