//#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_BUFFER_H_
//#define _IS_SDE_NVDATAPUBLISH_NETWORK_BUFFER_H_
//#include "../../Tools/SpatialDataBase.h"
//
//namespace IS_SDE
//{
//	namespace NVDataPublish
//	{
//		namespace Network
//		{
//			class FIFOBuffer : public IS_SDE::StorageManager::IBuffer
//			{
//			public:
//				FIFOBuffer(Tools::PropertySet& ps);
//				// String       Value		Description
//				// ----------------------------------------------------------------
//				// Capacity		VT_ULONG	FIFOBuffer maximum capacity.
//				// WriteThrough	VT_BOOL		Enable or disable write through policy.
//				// PageSize		VT_ULONG	specify the page size.
//
//				virtual ~FIFOBuffer();
//
//				//
//				// IStorageManager interface
//				//
//				virtual void loadByteArray(const id_type id, size_t& len, byte** data);
//				virtual void storeByteArray(id_type& id, const size_t len, const byte* const data);
//				virtual void deleteByteArray(const id_type id);
//
//				//
//				// IBuffer interface
//				//
//				virtual void clear();
//				virtual size_t getHits();
//
//			private:
//				class Entry
//				{
//				public:
//					Entry(const byte* const d, size_t len) : m_pData(0), m_bDirty(false)
//					{
//						m_pData = new byte[len];
//						memcpy(m_pData, d, len);
//					}
//
//					~Entry() { delete[] m_pData; }
//
//					byte* m_pData;
//					bool m_bDirty;
//				}; // Entry
//
//			private:
//				void addEntry(id_type id, Entry* pEntry);
//				void removeEntry();
//
//			private:
//				size_t m_capacity;
//				bool m_bWriteThrough;
//				IStorageManager* m_pStorageManager;
//				size_t m_pageSize;
//				std::map<id_type, Entry*> m_buffer;
//				std::queue<id_type> m_queue;
//				size_t m_hits;
//
//			}; // FIFOBuffer
//		}
//	}
//}
//
//#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_BUFFER_H_
