#ifndef _IS_SDE_SPATIALINDEX_NVRTREE_BULK_LOADER_H_
#define _IS_SDE_SPATIALINDEX_NVRTREE_BULK_LOADER_H_

#include "../../Tools/SmartPointer.h"
#include "../../Tools/Tools.h"
#include "../../Algorithms/IO/TemporaryFile.h"
#include "../Utility/BulkLoader.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		namespace NVRTree
		{
			class BulkLoadSource : public Tools::IObjectStream
			{
			public:
				BulkLoadSource(Tools::SmartPointer<IObjectStream> spStream, size_t howMany);
				BulkLoadSource(IObjectStream* pStream, size_t howMany);
				BulkLoadSource(IObjectStream* pStream);
				virtual ~BulkLoadSource();

				virtual Tools::IObject* getNext();
				virtual bool hasNext() throw ();
				virtual size_t size() throw (Tools::NotSupportedException);
				virtual void rewind() throw (Tools::NotSupportedException);

				Tools::SmartPointer<IObjectStream> m_spDataSource;
				size_t m_cHowMany;
			};	// BulkLoadSource

			class BulkLoader
			{
			public:
				void bulkLoadUsingSTR(
					NVRTree* pTree,
					IDataStream& stream,
					size_t bindex,
					size_t bleaf,
					size_t bufferSize);

			protected:
				void createLevel(
					NVRTree* pTree,
					Tools::IObjectStream& es,
					size_t dimension,
					size_t k,
					size_t b,
					size_t level,
					size_t bufferSize,
					UtiBL::TmpFile& tmpFile,
					size_t& numberOfNodes,
					size_t& totalData);

				Node* createNode(
					NVRTree* pTree,
					std::vector<Tools::SmartPointer<IData> >& e,
					size_t level);

				friend class BulkLoadSource;
			};
		}
	}
}

#endif // _IS_SDE_SPATIALINDEX_NVRTREE_BULK_LOADER_H_

