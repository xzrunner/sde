#include "String.h"
using namespace IS_SDE;

size_t WString::getByteArraySize() const
{
	return 
		sizeof(size_t) +
		sizeof(wchar_t) * m_wstr.size();
}

void WString::loadFromByteArray(const byte* data)
{
	size_t len;
	memcpy(&len, data, sizeof(size_t));
	data += sizeof(size_t);
	m_wstr.reserve(len);

	memcpy(&m_wstr, data, sizeof(wchar_t) * len);
	// data += sizeof(WCHAR) * len;
}

void WString::storeToByteArray(byte** data, size_t& length) const
{
	length = getByteArraySize();
	*data = new byte[length];

	byte* ptr = *data;
	size_t wsLen = m_wstr.size();
	memcpy(ptr, &wsLen, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_wstr, sizeof(wchar_t) * wsLen);
	// ptr += sizeof(WCHAR) * wsLen;
}

void WString::storeToByteArray(byte*& ptr) const
{
	size_t wsLen = m_wstr.size();
	memcpy(ptr, &wsLen, sizeof(size_t));
	ptr += sizeof(size_t);
	memcpy(ptr, &m_wstr, sizeof(wchar_t) * wsLen);
	// ptr += sizeof(WCHAR) * wsLen;
}