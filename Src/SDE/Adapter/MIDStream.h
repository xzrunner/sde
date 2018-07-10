//#ifndef _IS_SDE_ADAPTER_MIF_STREAM_H_
//#define _IS_SDE_ADAPTER_MIF_STREAM_H_
//#include "MIFAdapter.h"
//
//namespace IS_SDE
//{
//	namespace Adapter
//	{
//		class MIFStream : public IDataStream
//		{
//		public:
//			MIFStream(const std::wstring& filePath);
//			virtual ~MIFStream();
//
//			virtual IData* getNext();
//			virtual bool hasNext();
//			virtual size_t size();
//			virtual void rewind();
//
//		private:
//			std::ifstream m_fin;
//			SpatialIndex::Data* m_pNext;
//		};
//	}
//}
//
//#endif // _IS_SDE_ADAPTER_MIF_STREAM_H_