//#include <stdio.h>
//
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "NVRTree.h"
#include "Node.h"
#include "Leaf.h"
#include "Index.h"
#include "BulkLoader.h"
#include "../Utility/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SpatialIndex::NVRTree;
using namespace IS_SDE::SpatialIndex::UtiBL;

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
	if (m_cHowMany == 0 || ! m_spDataSource->hasNext()) return 0;
	m_cHowMany--;
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
// class BulkLoader
//

void BulkLoader::bulkLoadUsingSTR(
	SpatialIndex::NVRTree::NVRTree* pTree,
	IDataStream& stream,
	size_t bindex,
	size_t bleaf,
	size_t bufferSize)
{
	NodePtr n = pTree->readNode(pTree->m_rootID);
	pTree->deleteNode(n.get());

	// create the leaf level first.
	TmpFile* tmpFile = new TmpFile();
	size_t cNodes = 0;
	size_t cTotalData = 0;

#ifdef DEBUG
	std::cerr << "NVRTree::BulkLoader: Building level 0" << std::endl;
#endif

	createLevel(pTree, stream, pTree->DIMENSION, pTree->DIMENSION, bleaf, 0, bufferSize, *tmpFile, cNodes, cTotalData);

	pTree->m_stats.m_data = cTotalData;
	pTree->m_nextObjID = cTotalData;

	// create index levels afterwards.
	size_t level = 1;
	tmpFile->rewind();
	BulkLoadSource* bs = new BulkLoadSource(tmpFile);

	while (cNodes > 1)
	{
		cNodes = 0;
		TmpFile* pTF = new TmpFile();

		#ifndef NDEBUG
		std::cerr << "RTree::BulkLoader: Building level " << level << std::endl;
		#endif
		pTree->m_stats.m_nodesInLevel.push_back(0);

		createLevel(pTree, *bs, pTree->DIMENSION, pTree->DIMENSION, bindex, level, bufferSize, *pTF, cNodes, cTotalData);
		delete bs;

		level++;
		pTF->rewind();
		bs = new BulkLoadSource(pTF);
	}

	pTree->m_stats.m_treeHeight = level;

	delete bs;

	pTree->storeHeader();
}

void BulkLoader::createLevel(
	SpatialIndex::NVRTree::NVRTree* pTree,
	Tools::IObjectStream& stream,
	size_t dimension,
	size_t k,
	size_t b,
	size_t level,
	size_t bufferSize,
	TmpFile& tmpFile,
	size_t& numberOfNodes,
	size_t& totalData)
{
	BulkLoadComparator bc(dimension - k);
	Tools::SmartPointer<Tools::IObjectStream> es(Tools::externalSort(stream, bc, bufferSize));
	size_t r = es->size();
	totalData = r;

	if (k == dimension - 1)
	{
		// store new pages in storage manager and page information in temporary file.

		std::vector<Tools::SmartPointer<IData> > entries;

		while (es->hasNext())
		{
			entries.push_back(Tools::SmartPointer<IData>(static_cast<IData*>(es->getNext())));

			if (entries.size() == b)
			{
				Node* n = createNode(pTree, entries, level);
				pTree->writeNode(n);
				if (r <= b) pTree->m_rootID = n->m_identifier;
				numberOfNodes++;
				tmpFile.storeRecord(n->m_nodeMBR, n->m_identifier);
				entries.clear();
				delete n;
			}
		}

		if (! entries.empty())
		{
			Node* n = createNode(pTree, entries, level);
			pTree->writeNode(n);
			if (r <= b) pTree->m_rootID = n->m_identifier;
			numberOfNodes++;
			tmpFile.storeRecord(n->m_nodeMBR, n->m_identifier);
			entries.clear();
			delete n;
		}
	}
	else
	{
		size_t P = static_cast<size_t>(std::ceil(static_cast<double>(r) / static_cast<double>(b)));
		size_t D = static_cast<size_t>(std::ceil(std::pow(static_cast<double>(P), static_cast<double>(k - 1) / static_cast<double>(k))));

		while (es->hasNext()) // this will happen S = ceil[P^(1 / k)] times
		{
			BulkLoadSource bs(es, D * b);
			size_t cTotalData;
			createLevel(pTree, bs, dimension, k - 1, b, level, bufferSize, tmpFile, numberOfNodes, cTotalData);
		}
	}
}

Node* BulkLoader::createNode(SpatialIndex::NVRTree::NVRTree* pTree, std::vector<Tools::SmartPointer<IData> >& e, size_t level)
{
	Node* n;

	if (level == 0) n = new Leaf(pTree, -1);
	else n = new Index(pTree, -1, level);

	for (size_t cChild = 0; cChild < e.size(); cChild++)
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

	return n;
}

