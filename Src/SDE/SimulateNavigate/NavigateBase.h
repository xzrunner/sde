#ifndef _IS_SDE_SIMULATE_NAVIGATE_NAVIGATE_BASE_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NAVIGATE_BASE_H_
#include "../Tools/SpatialDataBase.h"
#include "../NVDataPublish/Base.h"
#include "../BasicType/Scope.h"

namespace IS_SDE
{
	class Line;

	namespace Render
	{
		class GDIRender;
	}

	namespace SimulateNavigate
	{
		//
		// Interfaces
		//

		interface IPointBindLine
		{
		public:
			virtual void bind(const MapPos2D& p, Line** line, uint64_t* id) const = 0;
			virtual ~IPointBindLine() {}
		}; // IPointBindLine

		struct DisplayProcessInfo
		{
			std::vector<NVDataPublish::STATIC_LAYER*>* m_layers;
			Render::GDIRender* m_render;

			DisplayProcessInfo() : m_layers(NULL), m_render(NULL) {}

			bool isValid() const { return m_render != NULL && m_render != NULL; }

		}; // DisplayProcessInfo

		struct BetweenTwoNodeSearchingInfo
		{
			size_t m_start, m_end;
			std::vector<uint64_t> m_roadIDs;
			DisplayProcessInfo m_disPro;
			double m_knownDis;

			BetweenTwoNodeSearchingInfo() {}
			BetweenTwoNodeSearchingInfo(size_t s, size_t e, double dis = Tools::DOUBLE_MAX) 
				: m_start(s), m_end(e), m_knownDis(dis) {}

		}; // BetweenTwoNodeSearchingInfo

		struct SingleRouteSearchingInfo
		{
			struct LocationInfo
			{
				MapPos2D m_realPos;
				MapPos2D m_bindPos;
				bool m_bDir;

				LocationInfo(const MapPos2D& pos) : m_realPos(pos) {}
			}; // LocationInfo


			LocationInfo m_start, m_end;
			std::vector<uint64_t> m_roadIDs;
			DisplayProcessInfo m_disPro;
			bool m_bAStar;
			size_t m_expandNum;

			SingleRouteSearchingInfo(const MapPos2D& s, const MapPos2D& e) 
				: m_start(s), m_end(e), m_bAStar(true) {}

		}; // SingleRouteSearchingInfo

		struct MultiRoutesSearchingInfo
		{
			const std::vector<size_t>& m_multiSrc;
			std::set<uint64_t>& m_roadIDs;
			const MapScope& m_scope;

			MultiRoutesSearchingInfo(const std::vector<size_t>& nodes, std::set<uint64_t>& roadIDs, 
				const MapScope& scope) : m_multiSrc(nodes), m_roadIDs(roadIDs), m_scope(scope) {} 

		}; // MultiRoutesSearchingInfo

		interface INetWork
		{
		public:
			virtual double getShortestDistance(size_t src, size_t dest) const = 0;
			virtual double computeShortestPath(SingleRouteSearchingInfo& info) const = 0;
			virtual void computeShortestPath(MultiRoutesSearchingInfo& info) const = 0;

			virtual ~INetWork() {}
		}; // INetWork
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NAVIGATE_BASE_H_