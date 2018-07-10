#include "BulkLoader.h"
#include "../../BasicType/Rect.h"
#include "Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::UtiBL;

//BulkLoadSourceTwoPart::BulkLoadSourceTwoPart(
//	Tools::SmartPointer<IObjectStream> spStream0, size_t howMany0, 
//	Tools::SmartPointer<IObjectStream> spStream1, size_t howMany1
//	) : m_spDataSource0(spStream0), m_cHowMany0(howMany0),
//		m_spDataSource1(spStream1), m_cHowMany1(howMany1),
//		BulkLoadSource(NULL, 0)
//{
//}
//
//Tools::IObject* BulkLoadSourceTwoPart::getNext()
//{
//	if (m_cHowMany1 == 0 || !m_spDataSource1->hasNext())
//		return 0;
//
//	if (m_cHowMany0 != 0 && m_spDataSource0->hasNext())
//	{
//		--m_cHowMany0;
//		return m_spDataSource0->getNext();
//	}
//	else
//	{
//		--m_cHowMany1;
//		return m_spDataSource1->getNext();
//	}
//}
//
//bool BulkLoadSourceTwoPart::hasNext() throw ()
//{
//	return (m_cHowMany1 != 0 && m_spDataSource1->hasNext());
//}

BulkLoadComparator::BulkLoadComparator(size_t d) : m_compareDimension(d)
{
}

BulkLoadComparator::~BulkLoadComparator()
{
}

int BulkLoadComparator::compare(Tools::IObject* o1, Tools::IObject* o2)
{
	IData* d1 = dynamic_cast<IData*>(o1);
	IData* d2 = dynamic_cast<IData*>(o2);

	IShape* s1;	d1->getShape(&s1);
	IShape* s2;	d2->getShape(&s2);
	Rect r1; s1->getMBR(r1);
	Rect r2; s2->getMBR(r2);

	int ret = 0;
	if (
		r1.m_pHigh[m_compareDimension] + r1.m_pLow[m_compareDimension] <
		r2.m_pHigh[m_compareDimension] + r2.m_pLow[m_compareDimension]
	) 
		ret = -1;
	else if (
		r1.m_pHigh[m_compareDimension] + r1.m_pLow[m_compareDimension] >
		r2.m_pHigh[m_compareDimension] + r2.m_pLow[m_compareDimension]
	) 
		ret = 1;

	delete s1;
	delete s2;

	return ret;
}

TmpFile::TmpFile() : m_pNext(0)
{
}

TmpFile::~TmpFile()
{
	if (m_pNext != 0) delete m_pNext;
}

void TmpFile::storeRecord(Rect& r, id_type id)
{
	size_t len = sizeof(id_type) + 2 * r.DIMENSION * sizeof(double);
	byte* data = new byte[len];
	byte* ptr = data;

	memcpy(ptr, &id, sizeof(id_type));
	ptr += sizeof(id_type);
	memcpy(ptr, r.m_pLow, r.DIMENSION * sizeof(double));
	ptr += r.DIMENSION * sizeof(double);
	memcpy(ptr, r.m_pHigh, r.DIMENSION * sizeof(double));

	m_tmpFile.storeNextObject(len, data);
	delete[] data;
}

void TmpFile::loadRecord(Rect& r, id_type& id)
{
	size_t len;
	byte* data;
	m_tmpFile.loadNextObject(&data, len);

	byte* ptr = data;
	memcpy(&id, ptr, sizeof(id_type));
	ptr += sizeof(id_type);

	r.loadFromByteArray(ptr);

	delete[] data;
}

IData* TmpFile::getNext()
{
	if (m_pNext == 0) return 0;

	IData* ret = m_pNext;

	try
	{
		Rect r;
		id_type id;
		loadRecord(r, id);
		m_pNext = new SpatialIndex::Data(0, 0, r, id);
	}
	catch (Tools::EndOfStreamException& e)
	{
		m_pNext = 0;
	}
	catch (...)
	{
		m_pNext = 0;
		throw;
	}

	return ret;
}

bool TmpFile::hasNext() throw ()
{
	return (m_pNext != 0);
}

size_t TmpFile::size() throw (Tools::NotSupportedException)
{
	throw Tools::NotSupportedException("Not supported yet.");
}

void TmpFile::rewind()
{
	Rect r;
	id_type id;

	if (m_pNext != 0)
	{
		delete m_pNext;
		m_pNext = 0;
	}

	m_tmpFile.rewindForReading();

	try
	{
		loadRecord(r, id);
		m_pNext = new Data(0, 0, r, id);
	}
	catch (Tools::EndOfStreamException& e)
	{
	}
}
