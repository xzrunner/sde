#include "Utility.h"
#include "../../../BasicType/Point.h"
#include "../../../BasicType/Line.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Algorithm;

void Utility::getPointsCoords(const std::vector<IShape*>& shapes, std::vector<MapPos2D>& coords)
{
	if (!coords.empty())
		coords.clear();
	coords.reserve(shapes.size());

	for (size_t i = 0; i < shapes.size(); ++i)
	{
		Point* p = dynamic_cast<Point*>(shapes[i]);

		if (!p)
		{
			assert(p);
			throw Tools::IllegalArgumentException("Utility::getPointsCoords: Error Shape.");
		}

		coords.push_back(MapPos2D(p->m_pCoords[0], p->m_pCoords[1]));
	}
}

void Utility::getLinesCoords(const std::vector<IShape*>& shapes, std::vector<std::vector<MapPos2D> >& coords)
{
	if (!coords.empty())
		coords.clear();
	coords.reserve(shapes.size());

	for (size_t i = 0; i < shapes.size(); ++i)
	{
		Line* l = dynamic_cast<Line*>(shapes[i]);

		if (!l)
		{
			assert(l);
			throw Tools::IllegalArgumentException("Utility::getPointsCoords: Error Shape.");
		}

		std::vector<MapPos2D> pos;
		l->getPoint(&pos);
		coords.push_back(pos);
	}
}

void Utility::createPointsShapes(const std::vector<MapPos2D>& coords, std::vector<IShape*>& shapes)
{
	if (!shapes.empty())
	{
		for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
		shapes.clear();
	}
	shapes.reserve(coords.size());

	for (size_t i = 0; i < coords.size(); ++i)
		shapes.push_back(new Point(coords[i]));
}

void Utility::createLinesShapes(const std::vector<std::vector<MapPos2D> >& coords, std::vector<IShape*>& shapes)
{
	if (!shapes.empty())
	{
		for_each(shapes.begin(), shapes.end(), Tools::DeletePointerFunctor<IShape>());
		shapes.clear();
	}
	shapes.reserve(coords.size());

	for (size_t i = 0; i < coords.size(); ++i)
		shapes.push_back(new Line(coords[i]));
}