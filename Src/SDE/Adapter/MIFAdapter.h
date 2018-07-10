#ifndef _IS_SDE_ADAPTER_MIF_ADAPTER_H_
#define _IS_SDE_ADAPTER_MIF_ADAPTER_H_
#include "../Tools/SpatialDataBase.h"
#include "MIFStructDef.h"
#include "../SpatialIndex/Utility/Data.h"
#include "../BasicType/Point.h"
#include "../BasicType/Line.h"
#include "../BasicType/Polygon.h"

namespace IS_SDE
{
	namespace Adapter
	{
		class MIF_IO
		{
		public:
			static void loadMIFHeader(std::ifstream& fin, MIF_Header* header);
			static void transFstreamToData(std::ifstream& finMif, /*std::ifstream& finMid, 
				const MIF_Header& header, const std::vector<size_t>& attrIndexes, */
				SpatialIndex::Data** d, id_type id);

		private:
			static MIF_Head_Field_enum searchField(const std::string& str);
			static MIF_Object_enum searchType(const std::string& str); 
			static void skipEmptyLines(std::ifstream& fin);

			static void loadMIFPoint(std::ifstream& mifFile, Point& p);
			static void loadMIFLine(std::ifstream& mifFile, Line& l);
			static void loadMIFPline(std::ifstream& mifFile, const std::string& s, Line& l);
			static void loadMIFPoly(std::ifstream& mifFile, Polygon& p);

		};	// class MIF_IO
	}
}

#endif // _IS_SDE_ADAPTER_MIF_ADAPTER_H_