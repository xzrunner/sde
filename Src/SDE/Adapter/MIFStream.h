#ifndef _IS_SDE_ADAPTER_MIF_STREAM_H_
#define _IS_SDE_ADAPTER_MIF_STREAM_H_
#include "MIFAdapter.h"

namespace IS_SDE
{
	namespace Adapter
	{
		class MIFStream : public IDataStream
		{
		public:
			MIFStream(const std::wstring& filePath/*, const std::vector<size_t>& attrIndexes*/);
			virtual ~MIFStream();

			virtual IData* getNext();
			virtual bool hasNext();
			virtual size_t size();
			virtual void rewind();

		private:
			MIF_Header			m_mifHeader;
			std::ifstream		m_finMif, m_finMid;
			SpatialIndex::Data* m_pNext;
		}; // MIFStream
	}
}

#endif // _IS_SDE_ADAPTER_MIF_STREAM_H_