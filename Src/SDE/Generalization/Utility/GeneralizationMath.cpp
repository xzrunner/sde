#include "GeneralizationMath.h"

using namespace IS_SDE;
using namespace IS_SDE::Generalization::Utility;

size_t GeneralizationMath::calTargetMapSymbolNum(size_t sourceNum, double scaleTimes)
{
	return static_cast<size_t>(sourceNum * scaleTimes);
}