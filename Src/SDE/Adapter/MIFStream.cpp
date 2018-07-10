#include "MIFStream.h"

using namespace IS_SDE;
using namespace IS_SDE::Adapter;

MIFStream::MIFStream(const std::wstring& filePath/*, const std::vector<size_t>& attrIndexes*/)
: m_pNext(NULL)
{
	m_finMif.open(filePath.c_str());
	if (!m_finMif)
		throw Tools::IllegalArgumentException("Mif file not found.");

	std::wstring midPath = filePath;
	midPath.at(midPath.size() - 1) = midPath.at(midPath.size() - 1) + L'd' - L'f';
	m_finMid.open(midPath.c_str());
	if (!m_finMid)
		throw Tools::IllegalArgumentException("Mid file not found.");

	MIF_IO::loadMIFHeader(m_finMif, &m_mifHeader);
	MIF_IO::transFstreamToData(m_finMif, /*m_finMid, m_mifHeader, attrIndexes, */&m_pNext, 0);
}

MIFStream::~MIFStream()
{
	if (m_pNext != NULL)
		delete m_pNext;
}

IData* MIFStream::getNext()
{
	if (m_pNext == NULL)
		return NULL;

	SpatialIndex::Data* ret = m_pNext;
	m_pNext = NULL;
	MIF_IO::transFstreamToData(m_finMif, &m_pNext, ret->m_id + 1);

	return ret;
}

bool MIFStream::hasNext() throw (Tools::NotSupportedException)
{
	return m_pNext != NULL;
}

size_t MIFStream::size() throw (Tools::NotSupportedException)
{
	throw Tools::NotSupportedException("MIFStream::size: Operation not supported.");
}

void MIFStream::rewind() throw (Tools::NotSupportedException)
{
	if (m_pNext != NULL)
	{
		delete m_pNext;
		m_pNext = NULL;
	}

	m_finMif.clear();
	m_finMif.seekg(0, std::ios::beg);

	MIF_IO::transFstreamToData(m_finMif, &m_pNext, 0);
}