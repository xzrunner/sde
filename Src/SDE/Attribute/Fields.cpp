#include "Fields.h"

using namespace IS_SDE::Attribute;

size_t Fields::getIndex(const std::string& fieldName) const
{
	if (m_cashe.first == fieldName)
		return m_cashe.second;

	std::map<std::string, size_t>::const_iterator itr = m_nameToIndex.find(fieldName);
	if (itr == m_nameToIndex.end())
		throw Tools::IllegalArgumentException(
		"Fields::getIndex: Not find the field name. "
		);

	m_cashe = std::make_pair(itr->first, itr->second);
	return itr->second;
}