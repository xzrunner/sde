#include "Row.h"
#include "Field.h"
#include "Fields.h"

using namespace IS_SDE::Attribute;

size_t StaticRow::getFieldNum() const
{
	return m_vals.size();
}

void StaticRow::getField(size_t index, IS_SDE::IField& val) const
{
	if (index >= m_vals.size())
		throw Tools::IndexOutOfBoundsException(index);

	m_vals.at(index).getVal(val);
}

void StaticRow::getField(const std::string& field, IS_SDE::IField& val) const
{
	size_t index = m_fields->getIndex(field);
	getField(index, val);
}