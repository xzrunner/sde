#include "Field.h"

using namespace IS_SDE::Attribute;

VariantType Field::getType() const
{
	return m_val.m_varType;
}

void Field::setVal(const Variant& val)
{
	if (m_val.m_varType != val.m_varType)
		throw Tools::IllegalArgumentException(
		"Field::setVal: different type. "
		);

	m_val.m_val = val.m_val;
}

void Field::getVal(IField& val) const
{
	val.setVal(m_val);
}