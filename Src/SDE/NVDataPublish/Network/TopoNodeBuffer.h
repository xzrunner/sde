#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_TOPO_NODE_BUFFER_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_TOPO_NODE_BUFFER_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Network
		{
			class Publish;

			class TopoNodeBuffer
			{
			public:
				TopoNodeBuffer(Publish& publish) : m_publish(publish) {}
				~TopoNodeBuffer();

				void addUnknownNode(const MapPos2D& dest, size_t src, uint64_t id, size_t len);
				void eraseUnknowNode(const MapPos2D& src, size_t nodeID);

				void complementTopoNodeInfo();

			private:
				class Related
				{
				public:
					Related(const size_t& nodeID, uint64_t roadID, size_t len)
						: m_nodeID(nodeID), m_roadID(roadID), m_roadLength(len) {}

				public:
					size_t m_nodeID;
					uint64_t m_roadID;
					size_t m_roadLength;

				}; // Related

			private:
				void complementTopoNodeInfo(size_t id, const std::vector<Related>& related);

			private:
				typedef std::map<MapPos2D, std::vector<Related>, PosCmp<double> > UNKNOWN_NODES;
				UNKNOWN_NODES m_nodes;

				Publish& m_publish;

			}; // TopoNodeBuffer
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_TOPO_NODE_BUFFER_H_