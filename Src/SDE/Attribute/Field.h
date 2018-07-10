#ifndef _IS_SDE_ATTRIBUTE_FIELD_H_
#define _IS_SDE_ATTRIBUTE_FIELD_H_
#include "../Tools/SpatialDataBase.h"

using Tools::Variant;
using Tools::VariantType;

namespace IS_SDE
{
	namespace Attribute
	{
		class Field : public IField
		{
		public:

			//
			// IField interface
			//
			virtual VariantType getType() const;
			virtual void setVal(const Variant& val);
			virtual void getVal(IField& val) const;

		private:
			Variant m_val;

		}; // Field
	}
}

#endif // _IS_SDE_ATTRIBUTE_FIELD_H_