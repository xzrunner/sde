#include "TXTAdapter.h"
#include "../BasicType/Point3D.h"

using namespace IS_SDE;
using namespace IS_SDE::Adapter;

void TXT_IO::transFstreamToData(std::ifstream& finTxt, SpatialIndex::Data** d, id_type id)
{
	std::string line;
	while (getline(finTxt, line))
	{
		std::stringstream streamLine(line);
		double coordinate[3];
		streamLine >> coordinate[0] >> coordinate[1] >> coordinate[2];
		Point3D pointTmp(coordinate);
		*d = new SpatialIndex::Data(&pointTmp, id);
		break;
	}
}