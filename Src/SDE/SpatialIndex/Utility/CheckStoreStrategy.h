//#ifndef _IS_SDE_SPATIALINDEX_CHECK_STORE_STRATEGY_H_
//#define _IS_SDE_SPATIALINDEX_CHECK_STORE_STRATEGY_H_
//#include "../../Tools/SpatialDataBase.h"
//
//namespace IS_SDE
//{
//	namespace SpatialIndex
//	{
//		class CheckStoreStrategy : public ICheckStoreStrategy
//		{
//		private:
//			std::queue<id_type> m_ids;
//		
//		public:
//			size_t m_indexNum, m_leafNum;
//			size_t m_indexPageNum, m_leafPageNum;
//			size_t m_indexSize, m_leafSize;
//
//		public:
//			CheckStoreStrategy();
//			void getNextEntry(const IStorageManager* sm, const IEntry& entry, id_type& nextEntry, bool& hasNext);
//
//		}; // CheckStoreStrategy
//	}
//}
//
//#endif // _IS_SDE_SPATIALINDEX_CHECK_STORE_STRATEGY_H_