#ifndef _IS_SDE_ADAPTER_MIF_STRUCT_DEF_H_
#define _IS_SDE_ADAPTER_MIF_STRUCT_DEF_H_
#include "../Tools/Tools.h"

namespace IS_SDE
{
	namespace Adapter
	{
		enum MIF_Head_Field_enum
		{ 
			MIF_VERSION, 
			MIF_CHARSET, 
			MIF_DELIMITER, 
			MIF_UNIQUE, 
			MIF_INDEX, 
			MIF_COORDSYS, 
			MIF_TRANSFORM, 
			MIF_COLUMNS, 
			NO_SUCH_HEAD_FIELD
		};

		enum MIF_Object_enum
		{ 
			MIF_POINT, 
			MIF_LINE, 
			MIF_PLINE, 
			MIF_MULPLINE,
			MIF_REGION, 
			MIF_TEXT,
			MIF_ARC,
			NO_SUCH_OBJECT 
		};

		struct MIF_Header
		{
			std::string delimiter;
		};
	}
}

#endif	// _IS_SDE_ADAPTER_MIF_STRUCT_DEF_H_
