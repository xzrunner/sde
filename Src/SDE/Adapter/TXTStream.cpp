#include "TXTStream.h"

using namespace IS_SDE;
using namespace IS_SDE::Adapter;

TXTStream::TXTStream(const std::wstring& filePath)
	: m_pNext(NULL)
{
	m_finTxt.open(filePath.c_str());
	if (!m_finTxt)
		throw Tools::IllegalArgumentException("Txt file not found.");

	TXT_IO::transFstreamToData(m_finTxt, &m_pNext, 0);
}

TXTStream::~TXTStream()
{
	if (m_pNext != NULL)
		delete m_pNext;
}

IData* TXTStream::getNext()
{
	if (m_pNext == NULL)
		return NULL;

	SpatialIndex::Data* ret = m_pNext;
	m_pNext = NULL;
	TXT_IO::transFstreamToData(m_finTxt, &m_pNext, ret->m_id + 1);

	return ret;
}

bool TXTStream::hasNext() throw (Tools::NotSupportedException)
{
	return m_pNext != NULL;
}

size_t TXTStream::size() throw (Tools::NotSupportedException)
{
	throw Tools::NotSupportedException("TXTStream::size: Operation not supported.");
}

void TXTStream::rewind() throw (Tools::NotSupportedException)
{
	if (m_pNext != NULL)
	{
		delete m_pNext;
		m_pNext = NULL;
	}

	m_finTxt.clear();
	m_finTxt.seekg(0, std::ios::beg);

	TXT_IO::transFstreamToData(m_finTxt, &m_pNext, 0);
}