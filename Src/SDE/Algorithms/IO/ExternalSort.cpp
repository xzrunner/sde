//#include <stdio.h>
#include "ExternalSort.h"
using namespace IS_SDE::Algorithms;

ExternalSort::PQEntry::PQEntry(
	ISerializable* pS,
	IObjectComparator* pC,
	SmartPointer<TemporaryFile>& f
	)
	: m_pRecord(pS), m_pComparator(pC), m_spFile(f)
{
}

ExternalSort::PQEntry::~PQEntry()
{
	delete m_pRecord;
}

bool ExternalSort::PQEntry::ascendingComparator::operator()(PQEntry* x, PQEntry* y) const
{
	if (x->m_pComparator != 0)
	{
		int ret = x->m_pComparator->compare(
			dynamic_cast<IObject*>(x->m_pRecord),
			dynamic_cast<IObject*>(y->m_pRecord));

		if (ret == 1) return true;
		return false;
	}
	else
	{
		IComparable* pX = dynamic_cast<IComparable*>(x->m_pRecord);
		IComparable* pY = dynamic_cast<IComparable*>(y->m_pRecord);

		if (pX == 0 || pY == 0)
			throw IllegalArgumentException(
			"ExternalSort::PQEntry::ascendingComparator: "
			"argument is not comparable."
			);

		if (*pX > *pY) return true;
		return false;
	}
}

ExternalSort::ExternalSort(	
	IObjectStream& source,
	size_t bufferSize
	)
	: m_cMaxBufferSize(bufferSize),
	m_bFitsInBuffer(false),
	m_cNumberOfSortedRecords(0),
	m_cNumberOfReturnedRecords(0),
	m_pExternalSource(&source),
	m_pTemplateRecord(0),
	m_pComparator(0)
{
	mergeRuns();
}

ExternalSort::ExternalSort(	
	IObjectStream& source,
	IObjectComparator& comp,
	size_t bufferSize
	)
	: m_cMaxBufferSize(bufferSize),
	m_bFitsInBuffer(false),
	m_cNumberOfSortedRecords(0),
	m_cNumberOfReturnedRecords(0),
	m_pExternalSource(&source),
	m_pTemplateRecord(0),
	m_pComparator(&comp)
{
	mergeRuns();
}

ExternalSort::~ExternalSort()
{
	if (m_pTemplateRecord != 0) delete m_pTemplateRecord;
}

void ExternalSort::initializeRuns(std::deque<SmartPointer<TemporaryFile> >& runs)
{
	bool bEOF = false;

	while (! bEOF)
	{
		while (m_buffer.size() < m_cMaxBufferSize)
		{
			IObject* o = m_pExternalSource->getNext();  

			if (o == 0)
			{
				bEOF = true;
				break;
			}

			ISerializable* pS = dynamic_cast<ISerializable*>(o);
			if (pS == 0)
			{
				delete o;
				throw IllegalStateException(
					"ExternalSort::initializeRuns: "
					"object is not serializable."
					);
			}

			m_cNumberOfSortedRecords++;

#ifdef DEBUG
			if (m_cNumberOfSortedRecords % 1000000 == 0)
				std::cerr
				<< "ExternalSort::initializeRuns: loaded "
				<< m_cNumberOfSortedRecords << std::endl;
#endif

			if (m_pTemplateRecord == 0)
				m_pTemplateRecord = o->clone();

			SmartPointer<TemporaryFile> tf;
			m_buffer.push(new PQEntry(pS, m_pComparator, tf));
		}

		if (bEOF && runs.size() == 0)
			m_bFitsInBuffer = true;

		if (! m_buffer.empty())
		{
			TemporaryFile* tf = new TemporaryFile();
			while (! m_buffer.empty())
			{
				PQEntry* pqe = m_buffer.top(); m_buffer.pop();
				tf->storeNextObject(pqe->m_pRecord);
				delete pqe;
			}

			tf->rewindForReading();
			SmartPointer<TemporaryFile> spF(tf);
			runs.push_back(spF);
		}
	}
}

void ExternalSort::mergeRuns()
{
	std::deque<SmartPointer<TemporaryFile> > newruns;
	std::deque<SmartPointer<TemporaryFile> > runs;
	initializeRuns(runs);

	while (runs.size() > 1)
	{
		TemporaryFile* output = new TemporaryFile();

		std::priority_queue<
			PQEntry*,
			std::vector<PQEntry*>,
			PQEntry::ascendingComparator> buffer;

		size_t cRun = 0, cMaxRun = 0;
		size_t len;
		byte* data;

		while (buffer.size() < m_cMaxBufferSize)
		{
			try
			{
				runs[cRun]->loadNextObject(&data, len);

				ISerializable* pS =
					dynamic_cast<ISerializable*>(m_pTemplateRecord->clone());
				pS->loadFromByteArray(data);
				delete[] data;
				buffer.push(new PQEntry(pS, m_pComparator, runs[cRun]));
			}
			catch (EndOfStreamException)
			{
				// if there are no more records in the file, do nothing.
			}

			cMaxRun = std::max(cRun, cMaxRun);
			cRun++;
			if (cRun == runs.size()) cRun = 0;
		}

		while (! buffer.empty())
		{
			PQEntry* pqe = buffer.top(); buffer.pop();
			output->storeNextObject(pqe->m_pRecord);

			try
			{
				pqe->m_spFile->loadNextObject(pqe->m_pRecord);
				buffer.push(pqe);
			}
			catch (EndOfStreamException)
			{
				// if there are no more records in the file, do nothing.
				delete pqe;
			}
			catch (...)
			{
				delete pqe;
				throw;
			}
		}

		output->rewindForReading();
		newruns.push_back(SmartPointer<TemporaryFile>(output));

		for (cRun = 0; cRun <= cMaxRun; cRun++) runs.pop_front();

		if (runs.size() <= 1)
		{
			runs.insert(runs.end(), newruns.begin(), newruns.end());
			newruns.clear();
		}
	}

	m_spSortedFile = runs[0];
	m_spSortedFile->rewindForReading();
}

IObject* ExternalSort::getNext()
{
	if (m_cNumberOfReturnedRecords == m_cNumberOfSortedRecords)
		return 0;
	else
	{
		m_cNumberOfReturnedRecords++;

		size_t len;
		byte* data;
		m_spSortedFile->loadNextObject(&data, len);

		ISerializable* ret = dynamic_cast<ISerializable*>(m_pTemplateRecord->clone());
		ret->loadFromByteArray(data);
		delete[] data;
		return dynamic_cast<IObject*>(ret);
	}
}

bool ExternalSort::hasNext() throw ()
{
	if (m_cNumberOfReturnedRecords == m_cNumberOfSortedRecords)
		return false;
	else
		return true;
}

size_t ExternalSort::size() throw ()
{
	return m_cNumberOfSortedRecords;
}

void ExternalSort::rewind()
{
	try
	{
		m_spSortedFile->rewindForReading();
		m_cNumberOfReturnedRecords = 0;
	}
	catch (...)
	{
		m_cNumberOfReturnedRecords = m_cNumberOfSortedRecords;
		throw;
	}
}
