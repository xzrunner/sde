#include "VoronoiByDelaunay.h"
#include "../../Tools/SpatialMath.h"
#include "../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::Algorithms;

typedef DelaunayTriangulation::Node		DT_NODE;
typedef DelaunayTriangulation::NodeCmp	DT_NODE_CMP;

VoronoiByDelaunay::VoronoiByDelaunay(const std::vector<MapPos2D>& src)
	: m_dt(src)
{
	initNodes();
}

VoronoiByDelaunay::~VoronoiByDelaunay()
{
	for_each(m_nodes.begin(), m_nodes.end(), Tools::DeletePointerFunctor<Node>());
}

void VoronoiByDelaunay::getAllNodePos(std::vector<MapPos2D>& pos) const
{
	const std::set<DT_NODE*, DT_NODE_CMP>& nodeSet = m_dt.getAllNodes();
	pos.reserve(nodeSet.size());
	std::set<DT_NODE*, DT_NODE_CMP>::const_iterator itr = nodeSet.begin();
	for ( ; itr != nodeSet.end(); ++itr) 
		pos.push_back((*itr)->m_pos);
}

void VoronoiByDelaunay::getVoronoiNodeBoundLine(std::vector<IShape*>& shapes) const
{
	shapes.reserve(m_nodes.size());
	std::set<Node*, NodeCmp>::const_iterator itr = m_nodes.begin();
	for ( ; itr != m_nodes.end(); ++itr)
		shapes.push_back((*itr)->m_shapeLine);
}

double VoronoiByDelaunay::getNodeExtendArea(const MapPos2D& n) const
{
	Node tmp(n);
	std::set<Node*, NodeCmp>::const_iterator itr = m_nodes.find(&tmp);
	if (itr == m_nodes.end())
		return 0;
	else
		return (*itr)->m_area;
}

bool VoronoiByDelaunay::insertNode(const MapPos2D& p)
{
	if (m_dt.insertNode(p))
	{
		initNodes();
		return true;
	}
	else
		return false;
}

//
// VoronoiByDelaunay::Node 
//

VoronoiByDelaunay::Node::Node(D_NODE* n)
	: m_center(n->m_pos)
{
	std::vector<const D_TRI*> tris;
	n->getSurroundTris(tris);
	for (size_t i = 0; i < tris.size(); ++i)
	{
		std::vector<MapPos2D> pos;
		tris[i]->getNodesPos(pos);
		MapPos2D p;
		Tools::SpatialMath::getTriangleCircumcenter(pos, &p);
		m_bound.push_back(p);
	}

	std::vector<MapPos2D> pos(m_bound);
	pos.push_back(pos.front());
	m_shapeLine = new Line(pos);

	m_area = Tools::SpatialMath::calPolygonArea(pos);
}

VoronoiByDelaunay::Node::~Node()
{
	delete m_shapeLine;
}

//
// Internal 
//

void VoronoiByDelaunay::initNodes()
{
	if (!m_nodes.empty())
	{
		for_each(m_nodes.begin(), m_nodes.end(), Tools::DeletePointerFunctor<Node>());
		m_nodes.clear();
	}

	const std::set<D_NODE*, D_NODE_CMP>& nodes(m_dt.getAllNodes());
	std::set<D_NODE*, D_NODE_CMP>::const_iterator itr = nodes.begin();
	for ( ; itr != nodes.end(); ++itr)
		m_nodes.insert(new Node(*itr));
}