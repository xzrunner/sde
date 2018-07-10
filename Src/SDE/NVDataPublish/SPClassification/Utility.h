#ifndef _IS_SDE_NVDATAPUBLISH_SPC_UTILITY_H_
#define _IS_SDE_NVDATAPUBLISH_SPC_UTILITY_H_
#include "../Base.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SingleLayerNW;
		}
	}

	namespace NVDataPublish
	{
		namespace SPC
		{
			typedef SimulateNavigate::Network::SingleLayerNW SingleLayerNW;

			class Utility
			{
			public:
				static void getCondensedRoadIDsBySearchEachPair(const std::vector<STATIC_LAYER*>& layers, const Rect& scope, 
					const SingleLayerNW* nw, std::set<uint64_t>& roadIDs, Render::GDIRender* render = NULL);
				static void getCondensedRoadIDsByExpandToOthers(const std::vector<STATIC_LAYER*>& layers, const Rect& scope, 
					const SingleLayerNW* nw, std::set<uint64_t>& roadIDs, Render::GDIRender* render = NULL);

				static void getLineByAddress(const std::vector<STATIC_LAYER*>* layers, uint64_t id, Line** l);

			private:
				//
				// SearchEachPair
				//

				static void getIntersectionPos(const std::vector<STATIC_LAYER*>& layers, 
					const Rect& r, std::vector<MapPos2D>& intersection);

				static void SearchEachPair(const std::vector<MapPos2D>& pos, const SingleLayerNW* nw, 
					std::set<uint64_t>& roadIDs);
				static void SearchEachPair(const std::vector<MapPos2D>& pos, const SingleLayerNW* nw, 
					std::set<uint64_t>& roadIDs, const std::vector<STATIC_LAYER*>& layers, Render::GDIRender* render);

				//
				// ExpandToOthers
				//

				static void getIntersectionNodeID(const std::vector<STATIC_LAYER*>& layers, 
					const SingleLayerNW* nw, const Rect& r, std::vector<size_t>& nodes);

				static void ExpandToOthers(const std::vector<size_t>& nodeIDs, const Rect& r,
					const SingleLayerNW* nw, std::set<uint64_t>& roadIDs);

			private:
				static const double MIN_DIS_SQUARE;

			}; // Utility
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_SPC_UTILITY_H_