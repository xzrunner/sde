#include "QuadTree.h"
#include "Leaf.h"
#include "Index.h"
#include "BulkLoader.h"
#include "../../BasicType/Line.h"
#include "../../Tools/SpatialMath.h"
using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::QuadTree;
using namespace IS_SDE::SpatialIndex::UtiBL;

StoreObjs::StoreObjs() 
	: m_pTemplateObj(NULL), m_cNumOfShouldRefetchedObjs(0)
{
	m_shouldRefetchedObjs = new Algorithms::TemporaryFile;
		// delete at class RefetchObjs
}

StoreObjs::~StoreObjs() 
{
}

void StoreObjs::storeNextObject(IObject* obj)
{
	ISerializable* s = dynamic_cast<ISerializable*>(obj);
	m_shouldRefetchedObjs->storeNextObject(s);
	++m_cNumOfShouldRefetchedObjs;

	if (m_pTemplateObj == NULL)
		m_pTemplateObj = obj->clone();
}

RefetchObjs::RefetchObjs()
: m_objs(NULL), m_pTemplateObj(NULL), m_cNumOfRead(0), m_cTotNum(0)
{
}

RefetchObjs::RefetchObjs(RefetchObjs& refetchObjs)
	: m_objs(refetchObjs.m_objs), 
	m_pTemplateObj(refetchObjs.m_pTemplateObj),
	m_cNumOfRead(0),
	m_cTotNum(refetchObjs.m_cTotNum)
{
	if (m_cTotNum != 0 && m_pTemplateObj == NULL)
	{
		assert(0);
	}
}

RefetchObjs::~RefetchObjs()
{
	delete m_objs;
	delete m_pTemplateObj;
}

RefetchObjs& RefetchObjs::operator = (StoreObjs& storeObjs)
{
	m_objs = storeObjs.m_shouldRefetchedObjs;
	m_pTemplateObj = storeObjs.m_pTemplateObj;
	m_cNumOfRead = 0;
	m_cTotNum = storeObjs.m_cNumOfShouldRefetchedObjs;

	rewind();

	return *this;
}

IObject* RefetchObjs::getNext()
{
	if (m_cNumOfRead == m_cTotNum)
		return NULL;
	else
	{
		++m_cNumOfRead;

		size_t len;
		byte* data;
		m_objs->loadNextObject(&data, len);

		ISerializable* ret = dynamic_cast<ISerializable*>(m_pTemplateObj->clone());
		ret->loadFromByteArray(data);
		delete[] data;
		return dynamic_cast<IObject*>(ret);
	}
}

bool RefetchObjs::hasNext() throw ()
{
	if (m_cNumOfRead == m_cTotNum)
		return false;
	else
		return true;
}

size_t RefetchObjs::size() throw (NotSupportedException)
{
	throw NotSupportedException("SpatialIndex::RTree::BulkLoadSource::size: this should never be called.");
}

void RefetchObjs::rewind()
{
	m_objs->rewindForReading();
}

bool RefetchObjs::readComplete() const
{
	return m_cNumOfRead == m_cTotNum;
}

void RefetchObjs::clear()
{
	delete m_objs;
	m_objs = NULL;
	delete m_pTemplateObj;
	m_pTemplateObj = NULL;
}

BulkLoadSource::BulkLoadSource(SmartPointer<IObjectStream> spStream, double endCoor, bool isScaleX, RefetchObjs& lastObjs)
	: m_spDataSource(spStream), 
	m_pTemplateObj(NULL), 
	m_endCoor(endCoor), 
	m_scaleIsX(isScaleX),
	m_lastObjs(lastObjs)
{
	loadDataToBufferedObj();
}

BulkLoadSource::~BulkLoadSource()
{
}

IObject* BulkLoadSource::getNext()
{
	if (m_pTemplateObj == NULL)
		return m_pTemplateObj;

	IData* data = static_cast<IData*>(m_pTemplateObj);
	IShape* s; 
	data->getShape(&s);
	Rect r;
	s->getMBR(r);
	delete s;

	size_t compDim = m_scaleIsX ? 0 : 1;
	if (r.m_pLow[compDim] <= m_endCoor + Tools::DOUBLE_TOLERANCE)
	{
		IObject* ret = m_pTemplateObj;
		if (r.m_pHigh[compDim] >= m_endCoor - Tools::DOUBLE_TOLERANCE)
		{
			m_refetchObjs.storeNextObject(m_pTemplateObj);
			m_pTemplateObj = NULL;
		}
		loadDataToBufferedObj();
		return ret;
	}
	else
		return NULL;
}

bool BulkLoadSource::hasNext() throw ()
{
	if (m_pTemplateObj == NULL)
		return false;

	IData* data = static_cast<IData*>(m_pTemplateObj);
	IShape* s; 
	data->getShape(&s);
	Rect r;
	s->getMBR(r);
	delete s;

	size_t compDim = m_scaleIsX ? 0 : 1;
	if (r.m_pLow[compDim] <= m_endCoor + Tools::DOUBLE_TOLERANCE)
		return true;
	else
		return false;
}

size_t BulkLoadSource::size() throw (NotSupportedException)
{
	throw NotSupportedException("SpatialIndex::RTree::BulkLoadSource::size: this should never be called.");
}

void BulkLoadSource::rewind() throw (NotSupportedException)
{
	throw NotSupportedException("SpatialIndex::RTree::BulkLoadSource::rewind: this should never be called.");
}

void BulkLoadSource::loadDataToBufferedObj()
{
	IObject* pLastObj = m_lastObjs.getNext();
	if (pLastObj != NULL)
		m_pTemplateObj = pLastObj;
	else if (m_spDataSource->hasNext()) 
		m_pTemplateObj = m_spDataSource->getNext();
	else
		m_pTemplateObj = NULL;
}

void BulkLoadSource::readFinish()
{
	if (m_pTemplateObj != NULL)
		m_refetchObjs.storeNextObject(m_pTemplateObj);

	if (!m_lastObjs.readComplete())
		throw Tools::IllegalStateException("Not read complete. ");

	m_lastObjs.clear();
}

BulkLoadSourceMem::BulkLoadSourceMem(SmartPointer<IObjectStream> spStream, bool isScaleX)
	: m_spDataSource(spStream), 
	m_remainNumOfRefetchedObj(0), 
	m_itrRefetcher(m_refetcher.begin()), 
	m_isTemplateObjFromRefetcher(false),
	m_scaleIsX(isScaleX)
{
}

Tools::SmartPointer<IData> BulkLoadSourceMem::getNext()
{
	Tools::SmartPointer<IData> ret;
	if (m_pTemplateObj.get() == NULL)
		return ret;

	IData* data = m_pTemplateObj.get();
	IShape* s; 
	data->getShape(&s);
	Rect r;
	s->getMBR(r);
	delete s;

	const size_t COMP_DIM = m_scaleIsX ? 0 : 1;
	if (r.m_pLow[COMP_DIM] <= m_endCoor + Tools::DOUBLE_TOLERANCE)
	{
		ret = m_pTemplateObj;
		if (r.m_pHigh[COMP_DIM] >= m_endCoor - Tools::DOUBLE_TOLERANCE)
		{
			if (!m_isTemplateObjFromRefetcher)
				m_refetcher.push_back(m_pTemplateObj);
		}
		else
		{
			if (m_isTemplateObjFromRefetcher)
				m_itrRefetcher = m_refetcher.erase(--m_itrRefetcher);
		}
		loadDataToTemplateObj();
		return ret;
	}
	else
	{
		if (!m_isTemplateObjFromRefetcher)
			m_refetcher.push_back(m_pTemplateObj);

		bool shouldRefetch = m_remainNumOfRefetchedObj != 0;
		loadDataToTemplateObj();
		if (shouldRefetch)
			return getNext();
		return ret;
	}
}

bool BulkLoadSourceMem::isFinished() const
{
	return m_pTemplateObj.get() == NULL;
}

void BulkLoadSourceMem::loadDataToTemplateObj()
{
	if (m_remainNumOfRefetchedObj > 0)
	{
		m_pTemplateObj = *m_itrRefetcher;
		++m_itrRefetcher;
		--m_remainNumOfRefetchedObj;
		m_isTemplateObjFromRefetcher = true;
	}
	else if (m_spDataSource->hasNext())
	{
		m_pTemplateObj = Tools::SmartPointer<IData>(static_cast<IData*>(m_spDataSource->getNext()));
		m_isTemplateObjFromRefetcher = false;
	}
	else
	{
		Tools::SmartPointer<IData> nullData;
		m_pTemplateObj = nullData;
		m_isTemplateObjFromRefetcher = false;
	}
}

void BulkLoadSourceMem::setEndcoor(double endCoor)
{
	m_endCoor = endCoor;

	if (m_remainNumOfRefetchedObj != 0)
	{
		throw Tools::IllegalStateException(
			"BulkLoadSourceMem::setEndcoor: not fetch all. "
			);
	}

	m_remainNumOfRefetchedObj = m_refetcher.size();
	m_itrRefetcher = m_refetcher.begin();

	assert(!m_isTemplateObjFromRefetcher);
	if (m_pTemplateObj.get() == NULL)
		loadDataToTemplateObj();
}

void BulkLoader::bulkLoadUsingType0(
	SpatialIndex::QuadTree::QuadTree* pTree, 
	IDataStream& stream, 
	size_t leafCapacity, 
	size_t bufferSize
	)
{
	NodePtr n = pTree->readNode(pTree->m_rootID);
	pTree->deleteNode(n.get());

	// create the leaf level first.
	TmpFile* tmpFile = new TmpFile();
	size_t cLevel = 0;
	size_t cNodes = 0;
	size_t cTotalData = 0;

	Rect rootMBR;
	getTotReigon(pTree, stream, leafCapacity, &rootMBR, cLevel, cNodes, cTotalData);

	buildEmptyQuadTree(pTree, leafCapacity, bufferSize, cLevel, rootMBR);

	size_t cLeavesEachDim = static_cast<size_t>(pow(static_cast<double>(INDEX_CHILDREN_SIZE_EACH_DIM), static_cast<double>(cLevel - 1)));
	createLeafLevel(pTree, stream, leafCapacity, bufferSize, cLeavesEachDim, -1, rootMBR, true, cLevel != 1);

	condenseTree(pTree, cLevel, leafCapacity);

	delete tmpFile;
}

void BulkLoader::buildEmptyQuadTree(					
	SpatialIndex::QuadTree::QuadTree* pTree,
	size_t leafCapacity,
	size_t bufferSize,
	size_t cLevel,
	const Rect& rootMBR
	) const
{
	// [The Strategy of Decreasing Times of Reconstruct]
	Rect newRootMBR(rootMBR); newRootMBR.changeSize(2.0);
	if (cLevel != 1)
		++cLevel;

	size_t iStart = 0, iEnd = 0;
	for (size_t level = 0; level < cLevel; ++level)
	{
		bool isLeaf = level == cLevel - 1;
		for (size_t id = iStart; id <= iEnd; ++id)
		{
			NodePtr pNode;
			if (isLeaf)
			{
				pNode = pTree->m_leafPool.acquire();
				if (pNode.get() == 0)
					pNode = NodePtr(new Leaf(pTree, -1), &(pTree->m_leafPool));
			}
			else
			{
				pNode = pTree->m_indexPool.acquire();
				if (pNode.get() == 0)
					pNode = NodePtr(new Index(pTree, -1, level), &(pTree->m_indexPool));
			}
			pNode->m_level      = level;
			pNode->m_identifier = id;
			computeRegion(newRootMBR, id, &pNode->m_nodeMBR);

			if (!isLeaf)
			{
				for (size_t iChild = 0; iChild < INDEX_CHILDREN_SIZE; ++iChild)
				{
					Rect rChild;
					size_t idChild = id * INDEX_CHILDREN_SIZE + iChild + 1;
					pNode->setChildRegion(IndexChildPos(iChild), &rChild);
					pNode->insertEntry(0, 0, rChild, idChild);
				}
			}
			pTree->writeNode(pNode.get());
		}

		iStart = iStart * INDEX_CHILDREN_SIZE + 1;
		iEnd   = iEnd * INDEX_CHILDREN_SIZE + 4;
	}

}


void BulkLoader::getTotReigon(
	SpatialIndex::QuadTree::QuadTree* pTree,
	IDataStream& stream, 
	size_t leafCapacity,
	Rect* r,
	size_t& cLevel,
	size_t& cNodes,
	size_t& cTotData
	) const
{
	r->makeInfinite();
	cTotData = 0;
	while (stream.hasNext())
	{
		++cTotData;

		IData* srcData = stream.getNext();
		IShape* s; 
		srcData->getShape(&s);
		RectPtr mbr = pTree->m_rectPool.acquire();
		s->getMBR(*mbr);
		delete s;

		r->combineRect(*mbr);
	}

	pTree->m_nextObjID = cTotData;

	stream.rewind();

	size_t cLeaves = static_cast<size_t>(ceil(static_cast<double>(cTotData) / static_cast<double>(leafCapacity)));
	cLevel = static_cast<size_t>(ceil(log(static_cast<double>(cLeaves)) / log(static_cast<double>(INDEX_CHILDREN_SIZE)))) + 1;
		// level start from 0, so count layer will add 1 to level

	//// consider decreasing the number of new node at buildEmptyQuadTree
	//// so increase the time of split, decrease the time of condenseTree
	//if (cLevel != 1)
	//	cLevel -= 1;

	cNodes = (static_cast<size_t>(pow(static_cast<double>(INDEX_CHILDREN_SIZE), static_cast<double>(cLevel))) - 1) / 3;
		// numberOfNodes = 1 + 4 + 4^2 + 4^3 + ... + 4^cLevel
}

void BulkLoader::computeRegion(const Rect& src, size_t id, Rect* dest) const
{
	std::stack<byte> positions;
	
	while (id != 0)
	{
		size_t parentID = Utility::getParentID(id);
		assert(parentID != -1);
		positions.push(id - parentID * INDEX_CHILDREN_SIZE);
		id = parentID;
	}

	*dest = src;
	Rect tmp;
	while (!positions.empty())
	{
		tmp = *dest;
		byte position = positions.top(); positions.pop();
		Tools::SpatialMath::getRectChildGrid(tmp, position - 1, *dest);
	}
}

size_t BulkLoader::computeNodeID(size_t xIndex, size_t yIndex, size_t cLeavesEachDim) const
{
	std::vector<byte> positions;
	while (cLeavesEachDim != 1)
	{
		cLeavesEachDim = cLeavesEachDim >> 1;
		if (xIndex < cLeavesEachDim && yIndex >= cLeavesEachDim)
		{
			positions.push_back(1);
			yIndex -= cLeavesEachDim;
		}
		else if (xIndex >= cLeavesEachDim && yIndex >= cLeavesEachDim)
		{
			positions.push_back(2);
			xIndex -= cLeavesEachDim;
			yIndex -= cLeavesEachDim;
		}
		else if (xIndex < cLeavesEachDim && yIndex < cLeavesEachDim)
		{
			positions.push_back(3);
		}
		else if (xIndex >= cLeavesEachDim && yIndex < cLeavesEachDim)
		{
			positions.push_back(4);
			xIndex -= cLeavesEachDim;
		}
		else
			assert(0);
	}

	size_t ret = 0;
	for (size_t i = 0; i < positions.size(); ++i)
		ret = ret * INDEX_CHILDREN_SIZE + static_cast<size_t>(positions.at(i));

	return ret;
}

void BulkLoader::createLeafLevel(
	 SpatialIndex::QuadTree::QuadTree* pTree,
	 IObjectStream& stream,
	 size_t leafCapacity,
	 size_t bufferSize,
	 size_t cLeavesEachDim,
	 size_t xIndex,
	 const Rect& rootMBR,
	 bool compX,
	 bool needMoreLevel
	 ) const
{
	size_t compDim = compX ? 0 : 1;
	BulkLoadComparator bc(compDim);
	SmartPointer<IObjectStream> es(externalSort(stream, bc, bufferSize));

	if (!compX)
	{
		BulkLoadSourceMem bsMem(es, false);
		for (size_t yIndex = 0; yIndex < cLeavesEachDim; ++yIndex)
		{
			std::vector<Tools::SmartPointer<IData> > entries;

			double yScale = (rootMBR.m_pHigh[1] - rootMBR.m_pLow[1]) * (yIndex + 1) / cLeavesEachDim + rootMBR.m_pLow[1];
			bsMem.setEndcoor(yScale);

			Tools::SmartPointer<IData> data = bsMem.getNext();
			while (data.get() != NULL)
			{
				entries.push_back(data);
				data = bsMem.getNext();
			}
			
			if (!entries.empty())
			{
				// [The Strategy of Decreasing Times of Reconstruct]
				// Another method to compute new nodeID
				// old nodeID to new nodeID
				// 1, *, *... => 1, 4, *, *...
				// 2, *, *... => 2, 3, *, *...
				// 3, *, *... => 3, 2, *, *...
				// 4, *, *... => 4, 1, *, *...
				size_t nodeID;
				if (needMoreLevel)
					nodeID = computeNodeID(xIndex + cLeavesEachDim / 2, yIndex + cLeavesEachDim / 2, cLeavesEachDim * 2);
				else
					nodeID = computeNodeID(xIndex, yIndex, cLeavesEachDim);

				createNode(pTree, nodeID, entries, leafCapacity);
			}
		}
		assert(bsMem.isFinished());

		//////////////////////////////////////////////////////////////////////////

		//RefetchObjs willRefetchedObjs;
		//for (size_t yIndex = 0; yIndex < cLeavesEachDim; ++yIndex)
		//{
		//	std::vector<Tools::SmartPointer<IData> > entries;

		//	double yScale = (rootMBR.m_pHigh[1] - rootMBR.m_pLow[1]) * (yIndex + 1) / cLeavesEachDim + rootMBR.m_pLow[1];
		//	BulkLoadSource bs(es, yScale, false, willRefetchedObjs);

		//	while (bs.hasNext())
		//		entries.push_back(Tools::SmartPointer<IData>(static_cast<IData*>(bs.getNext())));

		//	bs.readFinish();
		//	willRefetchedObjs = bs.m_refetchObjs;
		//	// [The Strategy of Decreasing Times of Reconstruct]
		//	// Another method to compute new nodeID
		//	// old nodeID to new nodeID
		//	// 1, *, *... => 1, 4, *, *...
		//	// 2, *, *... => 2, 3, *, *...
		//	// 3, *, *... => 3, 2, *, *...
		//	// 4, *, *... => 4, 1, *, *...
		//	size_t nodeID;
		//	if (needMoreLevel)
		//		nodeID = computeNodeID(xIndex + cLeavesEachDim / 2, yIndex + cLeavesEachDim / 2, cLeavesEachDim * 2);
		//	else
		//		nodeID = computeNodeID(xIndex, yIndex, cLeavesEachDim);
		//	createNode(pTree, nodeID, entries, leafCapacity);
		//}
	}
	else
	{


		//////////////////////////////////////////////////////////////////////////

		RefetchObjs willRefetchedObjs; 
		for (size_t i = 0; i < cLeavesEachDim; ++i)
		{
			double xScale = (rootMBR.m_pHigh[0] - rootMBR.m_pLow[0]) * (i + 1) / cLeavesEachDim + rootMBR.m_pLow[0];
			BulkLoadSource bs(es, xScale, true, willRefetchedObjs);

			createLeafLevel(pTree, bs, leafCapacity, bufferSize, cLeavesEachDim, i, rootMBR, false, needMoreLevel);

			bs.readFinish();
			willRefetchedObjs = bs.m_refetchObjs;
		}
	}
}

void BulkLoader::createNode(
	SpatialIndex::QuadTree::QuadTree* pTree,
	id_type id,
	std::vector<Tools::SmartPointer<IData> >& e,
	size_t leafCapacity
	) const
{
	NodePtr n = pTree->readNode(id);
	if (e.size() <= leafCapacity)
	{
		for (size_t cChild = 0; cChild < e.size(); ++cChild)
		{
			size_t len;
			byte* data;
			e[cChild]->getData(len, &data);
			IShape* s; e[cChild]->getShape(&s);

			RectPtr mbr = pTree->m_rectPool.acquire();
			s->getMBR(*mbr);
			delete s;
			id_type id = e[cChild]->getIdentifier();
			n->insertEntry(len, data, *mbr, id);
		}
		pTree->writeNode(n.get());
	}
	else
	{
		id_type parentID = n->getIdentifier();

		NodePtr pNewParent = pTree->m_indexPool.acquire();
		if (pNewParent.get() == 0)
			pNewParent = NodePtr(new Index(pTree, -1, n->getLevel()), &(pTree->m_indexPool));
		pNewParent->m_level = n->m_level;
		pNewParent->m_identifier = parentID;
		pNewParent->m_nodeMBR = n->m_nodeMBR;

		for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
		{
			id_type childID = parentID * INDEX_CHILDREN_SIZE + i + 1;

			NodePtr pNode = pTree->m_leafPool.acquire();
			if (pNode.get() == 0)
				pNode = NodePtr(new Leaf(pTree, -1), &(pTree->m_leafPool));
			pNode->m_level = n->m_level + 1;
			pNode->m_identifier = childID;
			n->setChildRegion(static_cast<IndexChildPos>(i), &pNode->m_nodeMBR);

			pTree->writeNode(pNode.get());
			pNewParent->insertEntry(0, 0, pNode->m_nodeMBR, childID);
		}
		pTree->writeNode(pNewParent.get());

		std::vector<std::vector<Tools::SmartPointer<IData> > > eChildren(4);
		for (size_t cChild = 0; cChild < e.size(); ++cChild)
		{
			IShape* s; e[cChild]->getShape(&s);
			RectPtr mbr = pTree->m_rectPool.acquire();
			s->getMBR(*mbr);
			delete s;
			
			for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
				if (mbr->intersectsRect(*pNewParent->m_ptrMBR[i]))
					eChildren.at(i).push_back(e.at(cChild));
		}

		for (size_t i = 0; i < INDEX_CHILDREN_SIZE; ++i)
			createNode(pTree, pNewParent->m_pIdentifier[i], eChildren.at(i), leafCapacity);
	}
}

void BulkLoader::condenseTree(
	SpatialIndex::QuadTree::QuadTree* pTree, 
	size_t cLevel, 
	size_t leafCapacity
	) const
{
	// [The Strategy of Decreasing Times of Reconstruct]
	if (cLevel != 1)
		++cLevel;

	if (cLevel < 2)
		return;

	std::stack<std::pair<size_t, size_t> > layerIDRange;
		// store index layer ID range, not including leaf layer's
	std::pair<size_t, size_t> idRange = std::make_pair(0, 0);
	layerIDRange.push(idRange);
	for (size_t i = 1; i < cLevel - 1; ++i)
	{
		idRange.first  = idRange.first * INDEX_CHILDREN_SIZE + 1;
		idRange.second = idRange.second * INDEX_CHILDREN_SIZE + 4;
		layerIDRange.push(idRange);
	}

	for (size_t parentLevel = cLevel - 2; parentLevel != 0; --parentLevel)
	{
		std::pair<size_t, size_t> idRangeParent = layerIDRange.top(); layerIDRange.pop();
		for (size_t idParent = idRangeParent.first; idParent <= idRangeParent.second; ++idParent)
		{
			NodePtr pNodeParent = pTree->readNode(idParent);

			size_t childrenDataSize = 0;
			std::pair<size_t, size_t> idRangeChild 
				= std::make_pair(idParent * INDEX_CHILDREN_SIZE + 1, idParent * INDEX_CHILDREN_SIZE + 4);

			bool allChildIsLeaf = true;
			std::vector<NodePtr> pNodeChildren; pNodeChildren.reserve(4);
			for (size_t idChild = idRangeChild.first; idChild <= idRangeChild.second; ++idChild)
			{
				NodePtr pNodeChild = pTree->readNode(idChild);
				if (pNodeChild->isIndex())
				{
					allChildIsLeaf = false;
					continue;
				}
				pNodeChildren.push_back(pNodeChild);
				childrenDataSize += pNodeChild->m_children;
			}

			if (allChildIsLeaf && childrenDataSize <= leafCapacity)
			{
				NodePtr pNode = pTree->m_leafPool.acquire();
				if (pNode.get() == 0)
					pNode = NodePtr(new Leaf(pTree, -1), &(pTree->m_leafPool));
				pNode->m_level		= parentLevel;
				pNode->m_identifier = idParent;
				pNode->m_nodeMBR	= pNodeParent->m_nodeMBR;

				for (size_t iChild = 0; iChild < INDEX_CHILDREN_SIZE; ++iChild)
				{
					NodePtr pNodeChild = pNodeChildren.at(iChild);
					for (size_t i = 0; i < pNodeChild->m_children; ++i)
						pNode->insertEntry(pNodeChild->m_pDataLength[i], pNodeChild->m_pData[i], *pNodeChild->m_ptrMBR[i], pNodeChild->m_pIdentifier[i]);
					pTree->deleteNode(pNodeChild.get());
				}

				pTree->writeNode(pNode.get());

				// pNodeChildren's m_pData has inserted into pNode's, they have the same data
				// so should only call one destructor and omit the other
				pNode.relinquish();
			}
		}
	}
}