#include "Condense.h"
#include "Stream.h"
#include "Strategy.h"
#include "GeoToPrj.h"
#include "../../Render/MapLayer.h"
#include "../../SimulateNavigate/SpatialIndex/StaticRTree/StaticRTree.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Features;

CondenseData::CondenseData(
	Render::MapLayer* src, 
	size_t layerID, 
	Tools::PropertySet& ps
	) : 
	m_src(src->getSpatialIndex()), 
	m_layerID(layerID)
{
	Tools::Variant var;

	var = ps.getProperty("PageSize");
	if (var.m_varType != Tools::VT_ULONG)
		throw Tools::IllegalArgumentException(
		"CondenseData::CondenseData: Property PageSize must be Tools::VT_ULONG"
		);
	m_pageSize = var.m_val.ulVal;

	// Path root
	var = ps.getProperty("FilePath");
	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_PWCHAR)
			throw Tools::IllegalArgumentException(
			"CondenseData::CondenseData: Property FilePath must be Tools::VT_PWCHAR"
			);

		m_filePath = std::wstring(var.m_val.pwcVal) + L"\\" + src->getLayerName();
	}

	var = ps.getProperty("PrecisionTransToInt");
	if (var.m_varType != Tools::VT_ULONG)
		throw Tools::IllegalArgumentException(
		"CondenseData::CondenseData: Property PrecisionTransToInt must be Tools::VT_ULONG"
		);
	m_trans = new GeoToPrj(var.m_val.ulVal);

	var = ps.getProperty("CondenseStrategy");
	if (var.m_varType != Tools::VT_ULONG)
		throw Tools::IllegalArgumentException(
		"CondenseData::CondenseData: Property CondenseStrategy must be Tools::VT_ULONG"
		);
	CondenseStrategyType cs = static_cast<CondenseStrategyType>(var.m_val.ulVal);
	switch (cs)
	{
	case CS_NO_OFFSET:
		m_strategy = new NoOffsetCondenseStrategy;
		break;
	default:
		throw Tools::IllegalArgumentException("CondenseData: Unknown bulk condense strategy.");
		break;
	}
}

CondenseData::~CondenseData()
{
	delete m_trans;
	delete m_strategy;

	delete m_dest;
	delete m_file;
}

void CondenseData::bulkLoadData(size_t layerID)
{
	std::wstring idxPath = m_filePath.substr(0, m_filePath.find_last_of(L'_')) + L"_STATIC";
	m_file = StorageManager::createNewStaticDiskStorageManager(idxPath, m_pageSize);

	LayerStream stream(m_src, layerID, m_strategy);

	id_type indexIdentifier;

	ISpatialIndex* tree = SimulateNavigate::SpatialIndex::StaticRTree::
		createAndBulkLoadNewStaticRTree(SimulateNavigate::SpatialIndex::StaticRTree::BLM_STR, stream, *m_file, *this, indexIdentifier, m_strategy->getType());
	m_dest = dynamic_cast<STATIC_LAYER*>(tree);
	assert(m_dest);
}

size_t CondenseData::getLayerDataSizeAfterCondense() const
{
	CountCondenseSizeQueryStrategy qs(m_strategy);
	m_src->queryStrategy(qs);
	return qs.m_size;
}

size_t CondenseData::getNodeByteArraySize(size_t cChild)
{
	return
		SimulateNavigate::SpatialIndex::StaticRTree::Node::CHILDREN_COUNT_SIZE	+	// m_children
		SimulateNavigate::SpatialIndex::StaticRTree::Node::MBR_SIZE				+	// m_nodeMBR
		cChild * (
			SimulateNavigate::SpatialIndex::StaticRTree::Node::MBR_SIZE				+	// m_ptrMBR
			SimulateNavigate::SpatialIndex::StaticRTree::Node::EDGE_ID_SIZE			+	// m_pIdentifier
			SimulateNavigate::SpatialIndex::StaticRTree::Node::EACH_CHILD_SIZE_SIZE		// m_pDataLength
		);
}

//
// class CondenseData::CountCondenseSizeQueryStrategy
//

CondenseData::CountCondenseSizeQueryStrategy::CountCondenseSizeQueryStrategy(NVDataPublish::ICondenseStrategy* condenseStrategy)
	: m_condenseStrategy(condenseStrategy), m_size(0)
{
}

void CondenseData::CountCondenseSizeQueryStrategy::getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
{
	const INode* n = dynamic_cast<const INode*>(&entry);
	if (n == NULL)
		throw Tools::IllegalStateException(
		"CountCondenseSizeQueryStrategy::getNextEntry: Entry is not Node. "
		);

	m_size += getNodeByteArraySize(n->getChildrenCount());

	if (n->isIndex())
	{
		for (size_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
		{
			m_ids.push(n->getChildIdentifier(cChild));
		}
	}
	else
		m_size += m_condenseStrategy->allShapeDataSize(n);

	if (!m_ids.empty())
	{
		nextEntry = m_ids.front(); m_ids.pop();
		hasNext = true;
	}
	else
	{
		hasNext = false;
	}
}
