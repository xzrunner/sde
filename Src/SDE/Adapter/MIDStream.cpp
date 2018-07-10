//#include "MIFStream.h"
//
//using namespace IS_SDE;
//using namespace IS_SDE::Adapter;
//
//MIFStream::MIFStream(const std::wstring& filePath)
//: m_pNext(NULL)
//{
//	m_fin.open(filePath.c_str());
//	if (!m_fin)
//		throw Tools::IllegalArgumentException("Input file not found.");
//
//	MIF_IO::transFstreamToData(m_fin, &m_pNext, 0);
//}
//
//MIFStream::~MIFStream()
//{
//	if (m_pNext != NULL)
//		delete m_pNext;
//}
//
//IData* MIFStream::getNext()
//{
//	if (m_pNext == NULL)
//		return NULL;
//
//	SpatialIndex::Data* ret = m_pNext;
//	m_pNext = NULL;
//	MIF_IO::transFstreamToData(m_fin, &m_pNext, ret->m_id + 1);
//
//	return ret;
//}
//
//bool MIFStream::hasNext() throw (Tools::NotSupportedException)
//{
//	return m_pNext != NULL;
//}
//
//size_t MIFStream::size() throw (Tools::NotSupportedException)
//{
//	throw Tools::NotSupportedException("MIFStream::size: Operation not supported.");
//}
//
//void MIFStream::rewind() throw (Tools::NotSupportedException)
//{
//	if (m_pNext != NULL)
//	{
//		delete m_pNext;
//		m_pNext = NULL;
//	}
//
//	m_fin.clear();
//	m_fin.seekg(0, std::ios::beg);
//
//	MIF_IO::transFstreamToData(m_fin, &m_pNext, 0);
//}