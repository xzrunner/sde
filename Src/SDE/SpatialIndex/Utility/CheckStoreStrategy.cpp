//#include "CheckStoreStrategy.h"
//using namespace IS_SDE;
//using namespace IS_SDE::SpatialIndex;
//
//CheckStoreStrategy::CheckStoreStrategy()
//: m_indexNum(0), m_leafNum(0), m_indexPageNum(0), m_leafPageNum(0), m_indexSize(0), m_leafSize(0)
//{
//}
//
//void CheckStoreStrategy::getNextEntry(const IStorageManager* sm, const IEntry& entry, id_type& nextEntry, bool& hasNext)
//{
//	const INode* n = dynamic_cast<const INode*>(&entry);
//	if (n == NULL)
//		throw Tools::IllegalStateException(
//		"PageInfoQueryStrategy::getNextEntry: Entry is not Node. "
//		);
//
//	size_t nodeSize = 0, pageNum = 0;
//	if (n->isIndex())
//	{
//		for (size_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
//		{
//			m_ids.push(n->getChildIdentifier(cChild));
//		}
//
//		n->getIdentifier();
//		sm->getNodePageInfo(nextEntry, nodeSize, pageNum);
//			// cannot use n->getIdentifier() which id may be logical id
//		m_indexSize += nodeSize;
//		m_indexPageNum += pageNum;
//		++m_indexNum;
//	}
//	else
//	{
//		sm->getNodePageInfo(nextEntry, nodeSize, pageNum);
//			// cannot use n->getIdentifier() which id may be logical id
//		m_leafSize += nodeSize;
//		m_leafPageNum += pageNum;
//		++m_leafNum;
//	}
//
//	if (!m_ids.empty())
//	{
//		nextEntry = m_ids.front(); m_ids.pop();
//		hasNext = true;
//	}
//	else
//	{
//		hasNext = false;
//	}
//}