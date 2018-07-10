#include "BulkLoader.h"
#include "Condense.h"
#include "EdgeIDTransform.h"
#include "../../SpatialIndex/Utility/BulkLoader.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/Node.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"

// debug
#include "../../BasicType/Line.h"
//

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Features;

//
// class BulkLoadSource
//

BulkLoadSource::BulkLoadSource(
	Tools::SmartPointer<IObjectStream> spStream, size_t howMany
	) : m_spDataSource(spStream), m_cHowMany(howMany)
{
}

BulkLoadSource::BulkLoadSource(IObjectStream* pStream, size_t howMany)
	: m_spDataSource(pStream), m_cHowMany(howMany)
{
}

BulkLoadSource::BulkLoadSource(IObjectStream* pStream)
	: m_spDataSource(pStream), m_cHowMany(Tools::UINT32_MAX)
{
}

BulkLoadSource::~BulkLoadSource()
{
}

Tools::IObject* BulkLoadSource::getNext()
{
	if (m_cHowMany == 0 || ! m_spDataSource->hasNext()) 
		return 0;
	--m_cHowMany;
	return m_spDataSource->getNext();
}

bool BulkLoadSource::hasNext() throw ()
{
	return (m_cHowMany != 0 && m_spDataSource->hasNext());
}

size_t BulkLoadSource::size() throw (Tools::NotSupportedException)
{
	throw Tools::NotSupportedException("SpatialIndex::RTree::BulkLoadSource::size: this should never be called.");
}

void BulkLoadSource::rewind() throw (Tools::NotSupportedException)
{
	throw Tools::NotSupportedException("SpatialIndex::RTree::BulkLoadSource::rewind: this should never be called.");
}

//
// class LayerBulkLoader
//

void LayerBulkLoader::bulkLoadUsingSTR(				
	NVDataPublish::STATIC_LAYER* pTree,
	IDataStream& stream,
	CondenseData& condenseData,
	size_t bufferSize
	)
{
	SimulateNavigate::SpatialIndex::StaticRTree::NodePtr n = pTree->readNode(pTree->m_rootID);
	pTree->deleteNode(n.get());

	// create the leaf level first.
	SpatialIndex::UtiBL::TmpFile* tmpFile = new SpatialIndex::UtiBL::TmpFile();
	size_t cNodes = 0;

#ifdef DEBUG
	std::cerr << "LayerBulkLoader::BulkLoader: Building level 0" << std::endl;
#endif

	partitionFace(pTree, stream, condenseData, bufferSize, 0, *tmpFile, cNodes);

	// create index levels afterwards.
	size_t level = 1;
	tmpFile->rewind();
	BulkLoadSource* bs = new BulkLoadSource(tmpFile);

	while (cNodes > 1)
	{
		cNodes = 0;
		SpatialIndex::UtiBL::TmpFile* pTF = new SpatialIndex::UtiBL::TmpFile();

#ifndef NDEBUG
		std::cerr << "LayerBulkLoader::BulkLoader: Building level " << level << std::endl;
#endif

		partitionFace(pTree, *bs, condenseData, bufferSize, level, *pTF, cNodes);
		delete bs;

		level++;
		pTF->rewind();
		bs = new BulkLoadSource(pTF);
	}

	delete bs;

	pTree->storeHeader();
}

void LayerBulkLoader::partitionFace(
	NVDataPublish::STATIC_LAYER* pTree,
	Tools::IObjectStream& stream,
	CondenseData& condenseData,
	size_t bufferSize,
	size_t level,
	SpatialIndex::UtiBL::TmpFile& tmpFile,
	size_t& numberOfNodes
	)
{
	SpatialIndex::UtiBL::BulkLoadComparator bc(0);
	Tools::SmartPointer<Tools::IObjectStream> es(Tools::externalSort(stream, bc, bufferSize));

	size_t allDataSize;
	if (level == 0)
		allDataSize = condenseData.getLayerDataSizeAfterCondense();
	else
		allDataSize = SimulateNavigate::SpatialIndex::StaticRTree::Node::getIndexByteArraySize(es->size());

	size_t totGridsCount = static_cast<size_t>(std::ceil(static_cast<double>(allDataSize) / static_cast<double>(condenseData.m_pageSize)));
	size_t rowGridsCount = static_cast<size_t>(std::ceil(std::pow(static_cast<double>(totGridsCount), 0.5)));

	size_t eachSize = static_cast<size_t>(std::ceil(static_cast<double>(es->size()) / static_cast<double>(rowGridsCount)));

	while (es->hasNext())
	{
		BulkLoadSource bs(es, eachSize);
		partitionLine(pTree, bs, condenseData, bufferSize, level, tmpFile, numberOfNodes);
	}
}

void LayerBulkLoader::partitionLine(
	NVDataPublish::STATIC_LAYER* pTree,
	Tools::IObjectStream& stream,
	CondenseData& condenseData,
	size_t bufferSize,
	size_t level,
	SpatialIndex::UtiBL::TmpFile& tmpFile,
	size_t& numberOfNodes
	)
{
	SpatialIndex::UtiBL::BulkLoadComparator bc(1);
	Tools::SmartPointer<Tools::IObjectStream> es(Tools::externalSort(stream, bc, bufferSize));

	// store new pages in storage manager and page information in temporary file.

	std::vector<std::pair<Tools::SmartPointer<IData>, size_t> > entries;

	size_t nodeStoreSize = CHILDREN_COUNT_SIZE + MBR_SIZE;
	size_t perItemSize = MBR_SIZE + EdgeIDTransform::EDGE_ID_SIZE + EACH_CHILD_SIZE_SIZE;

	bool hasLastData = false;
	Tools::SmartPointer<IData> lastData;
	size_t lastDataSize;
	bool bSingleNode = true;

	while (es->hasNext() || hasLastData)
	{
		bool bOverCapacity = false;

		if (hasLastData)
		{
			entries.push_back(std::make_pair(Tools::SmartPointer<IData>(lastData), nodeStoreSize - MBR_SIZE));
			nodeStoreSize += lastDataSize;
			hasLastData = false;
			assert(nodeStoreSize <= condenseData.m_pageSize);
		}

		if (es->hasNext())
		{
			IData* d = static_cast<IData*>(es->getNext());

			entries.push_back(std::make_pair(Tools::SmartPointer<IData>(d), nodeStoreSize - MBR_SIZE));

			nodeStoreSize += perItemSize;
			if (level == 0)
			{
				IShape* s;
				d->getShape(&s);
				size_t tmpDataSize = condenseData.m_strategy->dataSize(s);
				assert(tmpDataSize > 0);
				nodeStoreSize += tmpDataSize;
				lastDataSize = perItemSize + tmpDataSize;
				delete s;
			}
			else
				lastDataSize = perItemSize;

			if (nodeStoreSize > condenseData.m_pageSize)
			{
				bSingleNode = false;

				assert(!hasLastData);
				lastData = entries.back().first; entries.pop_back();
				hasLastData = true;

				SimulateNavigate::SpatialIndex::StaticRTree::Node* n 
					= new SimulateNavigate::SpatialIndex::StaticRTree::Node(pTree, -1);
				n->m_identifier = pTree->writeNode(n);

				createNode(n, pTree, entries, condenseData.m_layerID, *condenseData.m_strategy, level, condenseData.m_trans);
				pTree->writeNode(n);

				numberOfNodes++;
				tmpFile.storeRecord(n->m_nodeMBR, n->m_identifier);

				entries.clear();
				delete n;

				nodeStoreSize = CHILDREN_COUNT_SIZE + MBR_SIZE;
			}
		}
	}

	if (! entries.empty())
	{
		SimulateNavigate::SpatialIndex::StaticRTree::Node* n 
			= new SimulateNavigate::SpatialIndex::StaticRTree::Node(pTree, -1);
		n->m_identifier = pTree->writeNode(n);

		createNode(n, pTree, entries, condenseData.m_layerID, *condenseData.m_strategy, level, condenseData.m_trans);
		pTree->writeNode(n);

		if (bSingleNode) 
			pTree->m_rootID = n->m_identifier;
		numberOfNodes++;
		tmpFile.storeRecord(n->m_nodeMBR, n->m_identifier);

		entries.clear();
		delete n;
	}
}

void LayerBulkLoader::createNode(
	SimulateNavigate::SpatialIndex::StaticRTree::Node* n,
	NVDataPublish::STATIC_LAYER* pTree,
	std::vector<std::pair<Tools::SmartPointer<IData>, size_t> >& e, 
	size_t layerID,
	const NVDataPublish::ICondenseStrategy& strategy,
	size_t level,
	const ICrt* trans
	)
{
	for (size_t cChild = 0; cChild < e.size(); cChild++)
	{
		IShape* s; e[cChild].first->getShape(&s);
		RectPtr mbr = pTree->m_rectPool.acquire();
		s->getMBR(*mbr);
		if (level == 0)
			mbr->transGeoToPrj(*trans);

		if (level == 0)
		{
			size_t len;
			byte* data;
			s->transGeoToPrj(*trans);
			strategy.storeToByteArray(s, &data, len);
			id_type id = EdgeIDTransform::encodeTopoEdgeID(layerID, n->getIdentifier(), e[cChild].second);
			n->insertEntry(len, data, *mbr, id);
		}
		else
			n->insertEntry(0, 0, *mbr, e[cChild].first->getIdentifier());

		delete s;
	}
}
