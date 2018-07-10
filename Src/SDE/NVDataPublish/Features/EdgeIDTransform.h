#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_EDGE_ID_TRANSFORM_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_EDGE_ID_TRANSFORM_H_
#include "../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Features
		{
			class EdgeIDTransform
			{
			public:
				static id_type getIDWithLayerInfo(id_type id, size_t layerID);

				static uint64_t encodeTopoEdgeID(size_t layerID, id_type nodeID, size_t offset);
				static void decodeTopoEdgeID(uint64_t id, size_t* layerID, id_type* nodeID, size_t* offset);

			public:
				// Edge ID struct:
				static const size_t LAYER_ID_SIZE		= 1;
				static const size_t GRID_ID_SIZE		= 5;
				static const size_t	DATA_OFFSET_SIZE	= 2;
				static const size_t EDGE_ID_SIZE		= 8;

				static const uint64_t MAX_LAYER_ID		= 0x00000000000000ff;
				static const uint64_t MAX_GRID_ID		= 0x000000ffffffffff;
				static const uint64_t MAX_DATA_OFFSET	= 0x000000000000ffff;
				static const uint64_t MAX_ORIGINAL_ID	= 0x00ffffffffffffff;

				static const uint64_t LAYER_ID_MASK		= 0xff00000000000000;
				static const uint64_t GRID_ID_MASK		= 0x00ffffffffff0000;
				static const uint64_t DATA_OFFSET_MASK	= 0x000000000000ffff;

			}; // EdgeIDTransform
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_EDGE_ID_TRANSFORM_H_
