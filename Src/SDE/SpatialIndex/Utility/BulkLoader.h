#ifndef _IS_SDE_SPATIALINDEX_UTILITY_BULK_LOADER_H_
#define _IS_SDE_SPATIALINDEX_UTILITY_BULK_LOADER_H_

#include "../../Tools/SmartPointer.h"
#include "../../Tools/SpatialDataBase.h"
#include "../../Algorithms/IO/TemporaryFile.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace UtiBL
		{
			//class BulkLoadSourceTwoPart : public BulkLoadSource
			//{
			//public:
			//	BulkLoadSourceTwoPart(Tools::SmartPointer<IObjectStream> spStream0, size_t howMany0,
			//		Tools::SmartPointer<IObjectStream> spStream1, size_t howMany1);

			//	virtual Tools::IObject* getNext();
			//	virtual bool hasNext() throw ();

			//private:
			//	Tools::SmartPointer<IObjectStream> m_spDataSource0, m_spDataSource1;
			//	size_t m_cHowMany0, m_cHowMany1;
			//};	// BulkLoadSourceTwoPart

			class BulkLoadComparator : public Tools::IObjectComparator
			{
			public:
				BulkLoadComparator(size_t d);
				virtual ~BulkLoadComparator();

				virtual int compare(Tools::IObject* o1, Tools::IObject* o2);

				size_t m_compareDimension;
			};	// BulkLoadComparator

			class TmpFile : public IDataStream
			{
			public:
				TmpFile();
				virtual ~TmpFile();

				void storeRecord(Rect& r, id_type id);
				void loadRecord(Rect& r, id_type& id);

				virtual IData* getNext();
				virtual bool hasNext() throw ();
				virtual size_t size()
					throw (Tools::NotSupportedException);
				virtual void rewind();

				IS_SDE::Algorithms::TemporaryFile m_tmpFile;
				IData* m_pNext;
			};	// TmpFile
		}
	}
}

#endif	// _IS_SDE_SPATIALINDEX_UTILITY_BULK_LOADER_H_
