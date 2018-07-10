#ifndef _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_TABLE_BUILDING_H_
#define _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_TABLE_BUILDING_H_
#include "../../SimulateNavigate/NavigateBase.h"
#include <windows.h>

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class VisitedNode;
			class VisitedList;
			class CandidateList;
			class SingleLayerNW;
		}
	}

	namespace NVDataPublish
	{
		namespace LookUpTable
		{
			typedef SimulateNavigate::Network::VisitedNode VisitedNode;
			typedef SimulateNavigate::Network::VisitedList VisitedList;
			typedef SimulateNavigate::Network::CandidateList CandidateList;

			class TableBuilding
			{
			public:
				TableBuilding(const ILookUp* lookUp) : m_lookUp(lookUp) {}
				~TableBuilding();

				void build(HWND hwnd, IStorageManager* nwStorage, IStorageManager* tableStorage, 
					size_t startPage);

				static double queryMinDistance(IStorageManager* sm, size_t startPage, size_t gridNum,
					size_t sGridID, size_t dGridID);

				static void loadTable(IStorageManager* sm, size_t startPage, size_t gridNum,
					size_t destGridID, byte** data);
				static double queryMinDistance(size_t srcGridID, const byte* table);

			private:
				void initAllGrids(IStorageManager* sm);
				void uniqueGridsData();
				void initPairNodeToGrid();

				void computeTableDataDirectlySearch(HWND hwnd);

				void computeTableDataFastExpend(HWND hwnd);
				void updateMinDis(size_t fNode, double dis, size_t tGridID);
				void circleExpand(VisitedNode* n, VisitedList& visited, CandidateList& candidate) const;

				void outputToStorage(IStorageManager* sm, size_t startPage) const;

				static uint16_t encodeDis(double len);
				static size_t decodeDis(uint16_t len);

			private:
				typedef size_t									TOPO_NODE_ID;
				typedef size_t									TABLE_GRID_ID;
				typedef std::pair<TOPO_NODE_ID, TABLE_GRID_ID>	INNODE_TO_GRID;

				static const size_t MAX_DIS			= 0xffff;
				static const size_t DIS_SIZE		= 2;

				static const size_t DIS_16KM		= 0x00003fff;
				static const size_t DIS_128KM		= DIS_16KM << 3;
				static const size_t DIS_1024KM		= DIS_128KM << 3;
				static const size_t DIS_8192KM		= DIS_1024KM << 3;

				static const uint16_t DIS_16CARRY	= 0x0000;
				static const uint16_t DIS_128CARRY	= 0x4000;
				static const uint16_t DIS_1024CARRY	= 0x8000;
				static const uint16_t DIS_8192CARRY	= 0xC000;

				static const uint16_t DIS_16GAP		= 0x3fff;
				static const uint16_t DIS_128GAP	= 0x7fff;
				static const uint16_t DIS_1024GAP	= 0xBfff;
				static const uint16_t DIS_8192GAP	= 0xffff;
			private:
				class Grid
				{
				public:
					std::vector<double> m_minDisToOthers;
						// The order accord with BuildingTable's  m_grids

					std::vector<size_t> m_outNode, m_inNode;

					Grid(size_t c) {
						m_minDisToOthers.resize(c, Tools::DOUBLE_MAX);
					}

				}; // Grid

				class PairNodeGridCmp
				{
				public:
					bool operator() (const INNODE_TO_GRID& lhs, const INNODE_TO_GRID& rhs) const {
						return lhs.first < rhs.first;
					}
				} pairNodeGridCmp;

			private:
				const ILookUp* m_lookUp;
				SimulateNavigate::Network::SingleLayerNW* m_network;

				std::vector<Grid*> m_grids;
					// The order accord with RegularGrid's GridID
					// From left to right, from low to top

				//std::map<TOPO_NODE_ID, TABLE_GRID_ID> m_inNodeToGrid;

				std::vector<INNODE_TO_GRID> m_inNodeToGrid;

			}; // TableBuilding
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_TABLE_BUILDING_H_
