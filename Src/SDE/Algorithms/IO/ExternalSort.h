#ifndef _IS_SDE_IO_ALGORITHMS_EXTERNALSORT_H_
#define _IS_SDE_IO_ALGORITHMS_EXTERNALSORT_H_
#include "../../Tools/Tools.h"
#include "TemporaryFile.h"

using namespace Tools;

namespace IS_SDE
{
	namespace Algorithms
	{
		class ExternalSort : public IObjectStream
		{
		public:
			ExternalSort(
				IObjectStream& source,
				size_t bufferSize
				);
			ExternalSort(
				IObjectStream& source,
				IObjectComparator& comp,
				size_t bufferSize
				);
			virtual ~ExternalSort();

			virtual IObject* getNext();
			virtual bool hasNext() throw();
			virtual size_t size() throw();
			virtual void rewind();

		private:
			class PQEntry
			{
			public:
				PQEntry(
					ISerializable* pS,
					IObjectComparator* pC,
					SmartPointer<TemporaryFile>& f
					);
				~PQEntry();

				struct ascendingComparator
					: public std::binary_function<
					PQEntry*, PQEntry*, bool
					>
				{
					bool operator()
						(
						PQEntry* x,
						PQEntry* y
						) const;
				};

				ISerializable* m_pRecord;
				IObjectComparator* m_pComparator;
				SmartPointer<TemporaryFile> m_spFile;
			};	// PQEntry

			void initializeRuns(std::deque<SmartPointer<TemporaryFile> >& runs);
			void mergeRuns();

			std::priority_queue<
				PQEntry*,
				std::vector<PQEntry*>,
				PQEntry::ascendingComparator> m_buffer;

			size_t m_cMaxBufferSize;
			bool m_bFitsInBuffer;
			size_t m_cNumberOfSortedRecords;
			size_t m_cNumberOfReturnedRecords;

			IObjectStream* m_pExternalSource;
			IObject* m_pTemplateRecord;
			IObjectComparator* m_pComparator;
			SmartPointer<TemporaryFile> m_spSortedFile;
		}; // ExternalSort
	}
}

#endif // _IS_SDE_IO_ALGORITHMS_EXTERNALSORT_H_

