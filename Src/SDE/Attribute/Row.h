#ifndef _IS_SDE_ATTRIBUTE_ROW_H_
#define _IS_SDE_ATTRIBUTE_ROW_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Attribute
	{
		class Fields;
		class Field;

		class StaticRow : public IRow
		{
		public:

			//
			// ISerializable interface
			//
			virtual size_t getByteArraySize() const;
			virtual void loadFromByteArray(const byte* data);
			virtual void storeToByteArray(byte** data, size_t& length) const;

			//
			// IRow interface
			//
			virtual size_t getFieldNum() const = 0;
			virtual void getField(size_t index, IField& val) const = 0;
			virtual void getField(const std::string& field, IField& val) const = 0;
			
		private:
			const Fields*	   m_fields;
			std::vector<Field> m_vals;

		}; // Row
	}
}

#endif // _IS_SDE_ATTRIBUTE_ROW_H_