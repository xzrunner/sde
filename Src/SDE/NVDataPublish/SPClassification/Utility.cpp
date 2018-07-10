#include "Utility.h"
#include "../../NVDataPublish/Features/EdgeIDTransform.h"
#include "../../SpatialIndex/Utility/Visitor.h"
#include "../../BasicType/Line.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"
#include "../../SimulateNavigate/Network/SingleLayerNW.h"
#include "../../Tools/SpatialMath.h"

#include "../../Render/Render.h"
#include "../../BasicType/Point.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::SPC;

const double Utility::MIN_DIS_SQUARE = 0.25;

void Utility::getCondensedRoadIDsBySearchEachPair(
	const std::vector<NVDataPublish::STATIC_LAYER*>& layers, const Rect& scope, 
	const SingleLayerNW* nw, std::set<uint64_t>& roadIDs, Render::GDIRender* render/* = NULL*/
	)
{
	std::vector<MapPos2D> boundPos;
	getIntersectionPos(layers, scope, boundPos);

	if (render == NULL)
		SearchEachPair(boundPos, nw, roadIDs);
	else
		SearchEachPair(boundPos, nw, roadIDs, layers, render);
}

void Utility::getCondensedRoadIDsByExpandToOthers(
	const std::vector<NVDataPublish::STATIC_LAYER*>& layers, const Rect& scope, 
	const SingleLayerNW* nw, std::set<uint64_t>& roadIDs, Render::GDIRender* render/* = NULL*/
	)
{
	std::vector<size_t> nodes;
	getIntersectionNodeID(layers, nw, scope, nodes);

	ExpandToOthers(nodes, scope, nw, roadIDs);
}

void Utility::getLineByAddress(const std::vector<NVDataPublish::STATIC_LAYER*>* layers, uint64_t id, Line** l)
{
	size_t layerID;
	id_type nodeID;
	size_t offset;
	NVDataPublish::Features::EdgeIDTransform::decodeTopoEdgeID(id, &layerID, &nodeID, &offset);

	IShape* s;
	assert(layerID < layers->size());
	layers->at(layerID)->readDataByAddress(nodeID, offset, &s);

	*l = dynamic_cast<Line*>(s);
}

void Utility::getIntersectionPos(const std::vector<NVDataPublish::STATIC_LAYER*>& layers, 
								 const Rect& r, std::vector<MapPos2D>& intersection)
{
	MapScope s(r);

	std::vector<MapPos2D> rectBound;
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMin));
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMax));
	rectBound.push_back(MapPos2D(s.m_xMax, s.m_yMax));
	rectBound.push_back(MapPos2D(s.m_xMax, s.m_yMin));
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMin));
	Line rectLine(rectBound);

	std::vector<IShape*> shapes;
	SpatialIndex::FetchDataVisitor vis(shapes);
	for (size_t i = 0; i < layers.size(); ++i)
		layers[i]->intersectsWithQuery(rectLine, vis, false);

	for (size_t i = 0; i < shapes.size(); ++i)
	{
		std::vector<MapPos2D> linePos;
		Line* l = dynamic_cast<Line*>(shapes[i]);
		assert(l);
		l->getPoint(&linePos);

		for (size_t i = 0; i < linePos.size() - 1; ++i)
			Tools::SpatialMath::getIntersectionSegByRect(linePos[i], linePos[i + 1], s, intersection);
	}

	for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
}


void Utility::SearchEachPair(const std::vector<MapPos2D>& pos, const SingleLayerNW* nw, std::set<uint64_t>& roadIDs)
{
	for (size_t i = 0; i < pos.size() - 1; ++i)
	{
		for (size_t j = i + i; j < pos.size(); ++j)
		{
			if (Tools::SpatialMath::getDistanceWithSquare(pos[i], pos[j]) < MIN_DIS_SQUARE)
				continue;

			SimulateNavigate::SingleRouteSearchingInfo info(pos[i], pos[j]);
			nw->computeShortestPath(info);

			for (size_t k = 0; k < info.m_roadIDs.size(); ++k)
				roadIDs.insert(info.m_roadIDs[k]);
		}
	}
}

void Utility::SearchEachPair(
	const std::vector<MapPos2D>& pos, const SingleLayerNW* nw, std::set<uint64_t>& roadIDs, 
	const std::vector<NVDataPublish::STATIC_LAYER*>& layers, Render::GDIRender* render
	)
{
	render->initReserveDC();
	render->storeDC();

	for (size_t i = 0; i < pos.size() - 1; ++i)
	{
		for (size_t j = i + i; j < pos.size(); ++j)
		{
			if (Tools::SpatialMath::getDistanceWithSquare(pos[i], pos[j]) < MIN_DIS_SQUARE)
				continue;

			render->restoreDC();

			render->initMemDC();
			Point s(pos[i]), e(pos[j]);
			render->render(s);
			render->render(e);
			render->endMemRender();

			render->storeDC();

			SimulateNavigate::SingleRouteSearchingInfo info(pos[i], pos[j]);
			info.m_disPro.m_layers = const_cast<std::vector<NVDataPublish::STATIC_LAYER*>*>(&layers);
			info.m_disPro.m_render = render;
			nw->computeShortestPath(info);

			for (size_t k = 0; k < info.m_roadIDs.size(); ++k)
				roadIDs.insert(info.m_roadIDs[k]);
		}
	}
}

void Utility::getIntersectionNodeID(const std::vector<NVDataPublish::STATIC_LAYER*>& layers, const SingleLayerNW* nw,
									const Rect& r, std::vector<size_t>& nodes)
{
	MapScope s(r);

	std::vector<MapPos2D> rectBound;
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMin));
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMax));
	rectBound.push_back(MapPos2D(s.m_xMax, s.m_yMax));
	rectBound.push_back(MapPos2D(s.m_xMax, s.m_yMin));
	rectBound.push_back(MapPos2D(s.m_xMin, s.m_yMin));
	Line rectLine(rectBound);

	std::vector<IShape*> shapes;
	SpatialIndex::FetchDataVisitor vis(shapes);
	for (size_t i = 0; i < layers.size(); ++i)
		layers[i]->intersectsWithQuery(rectLine, vis, false);

	for (size_t i = 0; i < shapes.size(); ++i)
	{
		std::vector<MapPos2D> linePos;
		Line* l = dynamic_cast<Line*>(shapes[i]);
		assert(l);
		l->getPoint(&linePos);

		if (Tools::SpatialMath::isPointInRect(linePos[0], s))
			nodes.push_back(nw->queryTopoNodeID(linePos[0]));
		if (Tools::SpatialMath::isPointInRect(linePos[linePos.size() - 1], s))
			nodes.push_back(nw->queryTopoNodeID(linePos[linePos.size() - 1]));
	}
	nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

	for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
}

void Utility::ExpandToOthers(const std::vector<size_t>& nodeIDs, const Rect& r,
							 const SingleLayerNW* nw, std::set<uint64_t>& roadIDs)
{
	MapScope scope(r);
	SimulateNavigate::MultiRoutesSearchingInfo info(nodeIDs, roadIDs, scope);

	nw->computeShortestPath(info);
}