#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_UTILITY_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_UTILITY_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Algorithm
		{
			class Utility
			{
			public:
				static void getPointsCoords(const std::vector<IShape*>& shapes, std::vector<MapPos2D>& coords);
				static void getLinesCoords(const std::vector<IShape*>& shapes, std::vector<std::vector<MapPos2D> >& coords);
				static void createPointsShapes(const std::vector<MapPos2D>& coords, std::vector<IShape*>& shapes);
				static void createLinesShapes(const std::vector<std::vector<MapPos2D> >& coords, std::vector<IShape*>& shapes);

			}; // Utility
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_UTILITY_H_