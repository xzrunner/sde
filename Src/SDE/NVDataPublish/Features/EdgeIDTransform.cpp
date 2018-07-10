#include "EdgeIDTransform.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Features;

id_type EdgeIDTransform::getIDWithLayerInfo(id_type id, size_t layerID)
{
	assert(layerID <= MAX_LAYER_ID && id <= MAX_ORIGINAL_ID);
	return (static_cast<uint64_t>(layerID) << (sizeof(id_type) - LAYER_ID_SIZE) * 8) | id;
}

uint64_t EdgeIDTransform::encodeTopoEdgeID(size_t layerID, id_type nodeID, size_t offset)
{
	assert(layerID <= MAX_LAYER_ID && nodeID <= MAX_GRID_ID && offset <= MAX_DATA_OFFSET);
	return static_cast<uint64_t>(layerID) << (GRID_ID_SIZE + DATA_OFFSET_SIZE) * 8 |
		nodeID << DATA_OFFSET_SIZE * 8 |
		offset;
}

void EdgeIDTransform::decodeTopoEdgeID(uint64_t id, size_t* layerID, id_type* nodeID, size_t* offset)
{
	*layerID = static_cast<size_t>((id & LAYER_ID_MASK) >> (GRID_ID_SIZE + DATA_OFFSET_SIZE) * 8);
	*nodeID = (id & GRID_ID_MASK) >> (DATA_OFFSET_SIZE * 8);
	*offset = static_cast<size_t>(id & DATA_OFFSET_MASK);
}