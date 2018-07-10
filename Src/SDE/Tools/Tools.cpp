#include "Tools.h"
#include "../Algorithms/IO/ExternalSort.h"

using namespace Tools;

IndexOutOfBoundsException::IndexOutOfBoundsException(size_t i)
{
	std::ostringstream s;
	s << "Invalid index " << i;
	m_error = s.str();
}

std::string IndexOutOfBoundsException::what()
{
	return "IndexOutOfBoundsException: " + m_error;
}

IllegalArgumentException::IllegalArgumentException(std::string s) : m_error(s)
{
}

std::string IllegalArgumentException::what()
{
	return "IllegalArgumentException: " + m_error;
}

IllegalStateException::IllegalStateException(std::string s) : m_error(s)
{
}

std::string IllegalStateException::what()
{
	return "IllegalStateException: " + m_error + "\nPlease contact "/* + PACKAGE_BUGREPORT*/;
}

EndOfStreamException::EndOfStreamException(std::string s) : m_error(s)
{
}

std::string EndOfStreamException::what()
{
	return "EndOfStreamException: " + m_error;
}

ResourceLockedException::ResourceLockedException(std::string s) : m_error(s)
{
}

std::string ResourceLockedException::what()
{
	return "ResourceLockedException: " + m_error;
}

InvalidPageException::InvalidPageException(size_t id)
{
	std::ostringstream s;
	s << "Unknown page id " << id;
	m_error = s.str();
}

std::string InvalidPageException::what()
{
	return "InvalidPageException: " + m_error;
}

NotSupportedException::NotSupportedException(std::string s) : m_error(s)
{
}

std::string NotSupportedException::what()
{
	return "NotSupportedException: " + m_error;
}

Variant::Variant() : m_varType(VT_EMPTY)
{
}

std::ostream& Tools::operator<<(std::ostream& os, const Tools::PropertySet& p)
{
	std::map<std::string, Variant>::const_iterator it;

	for (it = p.m_propertySet.begin(); it != p.m_propertySet.end(); it++)
	{
		if (it != p.m_propertySet.begin()) os << ", ";

		switch ((*it).second.m_varType)
		{
		case VT_LONG:
			os << (*it).first << ": " << (*it).second.m_val.lVal;
			break;
		case VT_LONGLONG:
			os << (*it).first << ": " << (*it).second.m_val.llVal;
			break;
		case VT_BYTE:
			os << (*it).first << ": " << (*it).second.m_val.bVal;
			break;
		case VT_SHORT:
			os << (*it).first << ": " << (*it).second.m_val.iVal;
			break;
		case VT_FLOAT:
			os << (*it).first << ": " << (*it).second.m_val.fltVal;
			break;
		case VT_DOUBLE:
			os << (*it).first << ": " << (*it).second.m_val.dblVal;
			break;
		case VT_CHAR:
			os << (*it).first << ": " << (*it).second.m_val.cVal;
			break;
		case VT_USHORT:
			os << (*it).first << ": " << (*it).second.m_val.uiVal;
			break;
		case VT_ULONG:
			os << (*it).first << ": " << (*it).second.m_val.ulVal;
			break;
		case VT_ULONGLONG:
			os << (*it).first << ": " << (*it).second.m_val.ullVal;
			break;
		case VT_BOOL:
			os << (*it).first << ": " << (*it).second.m_val.blVal;
			break;
		case VT_PCHAR:
			os << (*it).first << ": " << (*it).second.m_val.pcVal;
			break;
		case VT_PVOID:
			os << (*it).first << ": ?";
			break;
		case VT_EMPTY:
			os << (*it).first << ": empty";
			break;
		default:
			os << (*it).first << ": unknown";
		}
	}

	return os;
}

PropertySet::PropertySet(const byte* data)
{
	loadFromByteArray(data);
}

size_t PropertySet::getByteArraySize() const
{
	size_t size = sizeof(size_t);
	std::map<std::string, Variant>::const_iterator it;

	for (it = m_propertySet.begin(); it != m_propertySet.end(); ++it)
	{
		switch ((*it).second.m_varType)
		{
		case VT_SHORT:
			size += sizeof(int16_t);
			break;
		case VT_LONG:
			size += sizeof(int32_t);
			break;
		case VT_LONGLONG:
			size += sizeof(int64_t);
			break;
		case VT_BYTE:
			size += sizeof(byte);
			break;
		case VT_FLOAT:
			size += sizeof(float);
			break;
		case VT_DOUBLE:
			size += sizeof(double);
			break;
		case VT_CHAR:
			size += sizeof(char);
			break;
		case VT_USHORT:
			size += sizeof(uint16_t);
			break;
		case VT_ULONG:
			size += sizeof(uint32_t);
			break;
		case VT_ULONGLONG:
			size += sizeof(uint64_t);
			break;
		case VT_BOOL:
			size += sizeof(byte);
			break;
		default:
			throw NotSupportedException(
				"PropertySet::getSize: Unknown type."
				);
		}
		size += (*it).first.size() + 1 + sizeof(VariantType);
	}

	return size;
}


void PropertySet::loadFromByteArray(const byte* ptr)
{
	m_propertySet.clear();

	size_t numberOfProperties;
	memcpy(&numberOfProperties, ptr, sizeof(size_t));
	ptr += sizeof(size_t);

	Variant v;

	for (size_t cIndex = 0; cIndex < numberOfProperties; ++cIndex)
	{
		std::string s(reinterpret_cast<const char*>(ptr));
		ptr += s.size() + 1;
		memcpy(&(v.m_varType), ptr, sizeof(VariantType));
		ptr += sizeof(VariantType);

		switch (v.m_varType)
		{
		case VT_SHORT:
			int16_t s;
			memcpy(&s, ptr, sizeof(int16_t));
			ptr += sizeof(int16_t);
			v.m_val.iVal = s;
			break;
		case VT_LONG:
			int32_t l;
			memcpy(&l, ptr, sizeof(int32_t));
			ptr += sizeof(int32_t);
			v.m_val.lVal = l;
			break;
		case VT_LONGLONG:
			int64_t ll;
			memcpy(&ll, ptr, sizeof(int64_t));
			ptr += sizeof(int64_t);
			v.m_val.llVal = ll;
			break;
		case VT_BYTE:
			byte b;
			memcpy(&b, ptr, sizeof(byte));
			ptr += sizeof(byte);
			v.m_val.bVal = b;
			break;
		case VT_FLOAT:
			float f;
			memcpy(&f, ptr, sizeof(float));
			ptr += sizeof(float);
			v.m_val.fltVal = f;
			break;
		case VT_DOUBLE:
			double d;
			memcpy(&d, ptr, sizeof(double));
			ptr += sizeof(double);
			v.m_val.dblVal = d;
			break;
		case VT_CHAR:
			char c;
			memcpy(&c, ptr, sizeof(char));
			ptr += sizeof(char);
			v.m_val.cVal = c;
			break;
		case VT_USHORT:
			uint16_t us;
			memcpy(&us, ptr, sizeof(uint16_t));
			ptr += sizeof(uint16_t);
			v.m_val.uiVal = us;
			break;
		case VT_ULONG:
			uint32_t ul;
			memcpy(&ul, ptr, sizeof(uint32_t));
			ptr += sizeof(uint32_t);
			v.m_val.ulVal = ul;
			break;
		case VT_ULONGLONG:
			uint64_t ull;
			memcpy(&ull, ptr, sizeof(uint64_t));
			ptr += sizeof(uint64_t);
			v.m_val.ullVal = ull;
			break;
		case VT_BOOL:
			byte bl;
			memcpy(&bl, ptr, sizeof(byte));
			ptr += sizeof(byte);
			v.m_val.blVal = static_cast<bool>(bl);
			break;
		default:
			throw IllegalStateException(
				"PropertySet::PropertySet: Deserialization problem."
				);
		}

		m_propertySet.insert(std::pair<std::string, Variant>(s, v));
	}
}

void PropertySet::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	size_t numberOfProperties = m_propertySet.size();
	memcpy(ptr, &numberOfProperties, sizeof(size_t));
	ptr += sizeof(size_t);

	std::map<std::string, Variant>::const_iterator it;

	for (it = m_propertySet.begin(); it != m_propertySet.end(); ++it)
	{
		size_t strSize = (*it).first.size();
		memcpy(ptr, (*it).first.c_str(), strSize);
		ptr += strSize;
		*ptr = 0;
		++ptr;

		memcpy(ptr, &((*it).second.m_varType), sizeof(VariantType));
		ptr += sizeof(VariantType);

		switch ((*it).second.m_varType)
		{
		case VT_SHORT:
			memcpy(ptr, &((*it).second.m_val.iVal), sizeof(int16_t));
			ptr += sizeof(int16_t);
			break;
		case VT_LONG:
			memcpy(ptr, &((*it).second.m_val.lVal), sizeof(int32_t));
			ptr += sizeof(int32_t);
			break;
		case VT_LONGLONG:
			memcpy(ptr, &((*it).second.m_val.llVal), sizeof(int64_t));
			ptr += sizeof(int64_t);
			break;
		case VT_BYTE:
			memcpy(ptr, &((*it).second.m_val.bVal), sizeof(byte));
			ptr += sizeof(byte);
			break;
		case VT_FLOAT:
			memcpy(ptr, &((*it).second.m_val.fltVal), sizeof(float));
			ptr += sizeof(float);
			break;
		case VT_DOUBLE:
			memcpy(ptr, &((*it).second.m_val.dblVal), sizeof(double));
			ptr += sizeof(double);
			break;
		case VT_CHAR:
			memcpy(ptr, &((*it).second.m_val.cVal), sizeof(char));
			ptr += sizeof(char);
			break;
		case VT_USHORT:
			memcpy(ptr, &((*it).second.m_val.uiVal), sizeof(uint16_t));
			ptr += sizeof(uint16_t);
			break;
		case VT_ULONG:
			memcpy(ptr, &((*it).second.m_val.ulVal), sizeof(uint32_t));
			ptr += sizeof(uint32_t);
			break;
		case VT_ULONGLONG:
			memcpy(ptr, &((*it).second.m_val.ullVal), sizeof(uint64_t));
			ptr += sizeof(uint64_t);
			break;
		case VT_BOOL:
			byte bl;
			bl = (*it).second.m_val.blVal;
			memcpy(ptr, &bl, sizeof(byte));
			ptr += sizeof(byte);
			break;
		default:
			throw NotSupportedException(
				"PropertySet::getData: Cannot serialize a variant of this type."
				);
		}
	}

	assert(ptr == (*data) + length);
}

Variant PropertySet::getProperty(const std::string& property)
{
	std::map<std::string, Variant>::iterator it = m_propertySet.find(property);
	if (it != m_propertySet.end()) 
		return (*it).second;
	else 
		return Variant();
}

void PropertySet::setProperty(const std::string& property, Variant& v)
{
	m_propertySet.insert(std::pair<std::string, Variant>(property, v));
}

void PropertySet::removeProperty(const std::string& property)
{
	std::map<std::string, Variant>::iterator it = m_propertySet.find(property);
	if (it != m_propertySet.end()) 
		m_propertySet.erase(it);
}

#if HAVE_PTHREAD_H
Tools::SharedLock::SharedLock(pthread_rwlock_t* pLock)
: m_pLock(pLock)
{
	pthread_rwlock_rdlock(m_pLock);
}

Tools::SharedLock::~SharedLock()
{
	pthread_rwlock_unlock(m_pLock);
}

Tools::ExclusiveLock::ExclusiveLock(pthread_rwlock_t* pLock)
: m_pLock(pLock)
{
	pthread_rwlock_wrlock(m_pLock);
}

Tools::ExclusiveLock::~ExclusiveLock()
{
	pthread_rwlock_unlock(m_pLock);
}
#endif

Tools::IObjectStream* Tools::externalSort(IObjectStream& source, size_t bufferSize)
{
	return new IS_SDE::Algorithms::ExternalSort(source, bufferSize);
}

Tools::IObjectStream* Tools::externalSort(IObjectStream& source, IObjectComparator& comp, size_t bufferSize)
{
	return new IS_SDE::Algorithms::ExternalSort(source, comp, bufferSize);
}
