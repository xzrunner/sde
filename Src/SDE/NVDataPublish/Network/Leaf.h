#ifndef _IS_SDE_NVDATAPUBLISH_NETWORK_LEAF_H_
#define _IS_SDE_NVDATAPUBLISH_NETWORK_LEAF_H_
#include "Node.h"
#include "../../SimulateNavigate/NavigateBase.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SLeaf;
		}
	}

	namespace NVDataPublish
	{
		namespace Network
		{
			class AdjacencyList;

			class Leaf : public Node, public Tools::ISerializable
			{
			public:
				Leaf(const Rect& scope) : m_bBuilt(false), m_bEmpty(true), Node(scope) {}

				//
				// ISerializable interface
				//
				virtual size_t getByteArraySize() const;
				virtual void loadFromByteArray(const byte* data);
				virtual void storeToByteArray(byte** data, size_t& length) const;

				virtual void split(PartitionMap& pm);
				virtual bool isLeaf() { return true; }
				virtual void queryAdjacencyListID(const MapPos2D& p, std::vector<size_t>* IDs) const;
				virtual void setNodeCount() {}

				static void queryAdjacencyListID(const MapPos2D& p, const byte* data, std::vector<size_t>* IDs);
				static void getAllAdjacencyListIDs(const byte* data, size_t& startID, size_t& num);

				void setBuilt() { m_bBuilt = true; }

				void setID(size_t page, size_t offset);

				bool hasNoData() const { return m_bEmpty; }

			public:
				// Leaf info
				static const size_t GRID_ID_SIZE	= 4;
				static const size_t PARTITION_SIZE	= 1;

			private:
				bool m_bBuilt;
				bool m_bEmpty;

				// for index data
				size_t m_id;
				// for leaf data
				size_t m_firstGridID;
				std::vector<double> m_verPartition;
				std::vector<std::vector<double> > m_horPartition;

				friend class AdjacencyList;
				friend class PartitionMap;

				friend class SimulateNavigate::Network::SLeaf;

			}; // Leaf
		}
	}	
}

#endif // _IS_SDE_NVDATAPUBLISH_NETWORK_LEAF_H_
