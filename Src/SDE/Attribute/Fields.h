#ifndef _IS_SDE_ATTRIBUTE_FIELDS_H_
#define _IS_SDE_ATTRIBUTE_FIELDS_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Attribute
	{
		class Fields : IFields
		{
		public:

			//
			// IFieldw interface
			//
			virtual size_t getIndex(const std::string& fieldName) const;
		
		private:
			std::map<std::string, size_t>  m_nameToIndex;
			mutable std::pair<std::string, size_t> m_cashe;

		}; // Fields
	}
}

#endif // _IS_SDE_ATTRIBUTE_FIELDS_H_