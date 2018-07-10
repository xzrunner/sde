#ifndef _IS_SDE_ADAPTER_TXT_STREAM_H_
#define _IS_SDE_ADAPTER_TXT_STREAM_H_
#include "TXTAdapter.h"

namespace IS_SDE
{
	namespace Adapter
	{
		class TXTStream : public IDataStream
		{
		public:
			TXTStream(const std::wstring& filePath);
			virtual ~TXTStream();

			virtual IData* getNext();
			virtual bool hasNext();
			virtual size_t size();
			virtual void rewind();

		private:
			std::ifstream		m_finTxt;
			SpatialIndex::Data* m_pNext;
		}; // TXTStream
	}
}

#endif // _IS_SDE_ADAPTER_TXT_STREAM_H_