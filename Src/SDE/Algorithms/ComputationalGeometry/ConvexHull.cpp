#include "ConvexHull.h"
#include "../../Tools/SpatialMath.h"

using namespace IS_SDE;
using namespace IS_SDE::Algorithms;

void ConvexHull::getConvexHullPos(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest)
{
	if (src.size() < 3)
		return;

	std::vector<MapPos2D> sorted(src);
	sort(sorted.begin(), sorted.end(), PosCmp<double>());

	std::vector<MapPos2D> upper, lower;
	upper.push_back(sorted[0]);
	upper.push_back(sorted[1]);
	lower.push_back(sorted[sorted.size() - 1]);
	lower.push_back(sorted[sorted.size() - 2]);

	for (size_t i = 2; i < sorted.size(); ++i)
	{
		upper.push_back(sorted[i]);
		size_t c = upper.size() - 1;
		while (upper.size() >= 3 && !Tools::SpatialMath::isTurnRight(upper[c- 2], upper[c - 1], upper[c]))
		{
			upper.erase(----upper.end());
			--c;
		}
	}
	for (int i = sorted.size() - 3; i >= 0; --i)
	{
		lower.push_back(sorted[i]);
		size_t c = lower.size() - 1;
		while (lower.size() >= 3 && !Tools::SpatialMath::isTurnRight(lower[c- 2], lower[c - 1], lower[c]))
		{
			lower.erase(----lower.end());
			--c;
		}
	}

	dest.reserve(lower.size() + upper.size() - 2);
	copy(upper.begin(), upper.end(), back_inserter(dest));
	if (lower.size() > 2)
		copy(++lower.begin(), --lower.end(), back_inserter(dest));
}