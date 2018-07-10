#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_STREAM_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_STREAM_H_
#include "../Base.h"
#include "../../SpatialIndex/NVRTree/Utility.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		class Data;
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class LayerStream : public IDataStream
			{
			public:
				LayerStream(ISpatialIndex* layer, size_t layerID, const ICondenseStrategy* cs);
				virtual ~LayerStream();

				virtual IData* getNext();
				virtual bool hasNext();
				virtual size_t size();
				virtual void rewind();

			private:
				void setNextData();

			private:
				size_t m_layerID;
				SpatialIndex::NVRTree::NVRTree* m_layer;
				const ICondenseStrategy* m_condenseStrategy;

				SpatialIndex::Data* m_pNext;

				std::queue<id_type> m_ids;
				SpatialIndex::NVRTree::NodePtr m_currNode;
				size_t m_currIndex;

			}; // LayerStream
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_STREAM_H_
