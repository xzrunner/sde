#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SEARCHING_UTILITY_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SEARCHING_UTILITY_H_
#include "../../NVDataPublish/Base.h"
#include "../../Tools/SpatialMath.h"
#include "../NavigateBase.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			typedef size_t NODE_ID;
			typedef size_t NEAREST_DIS;

			class BindPoint;
			class SingleLayerNW;
			class VisitedNode;

			class DestInfo
			{
			public:
				DestInfo();
				~DestInfo();

				void initByNodePos(const BindPoint& p, const NVDataPublish::ILookUp* table, bool bPreLoadTable);
				void initByNodeID(const SingleLayerNW* nw, size_t nodeID, const NVDataPublish::ILookUp* table, bool bPreLoadTable);

				bool beHasLookUpTableData() const {
					return m_bGridID != Tools::UINT32_MAX 
						|| m_eGridID != Tools::UINT32_MAX 
						|| m_bTable != NULL || m_eTable != NULL;
				}

				double getMinDisToHere(const NVDataPublish::ILookUp* lookUp, const MapPos2D& srcPos) const {					
					if (m_bTable != NULL)
					{
						if (m_beginID == m_endID)
						{
							assert(m_beginLen == 0 && m_endLen == 0);
							return lookUp->queryMinDistance(srcPos, m_bTable);
						}
						else if (m_bTable == m_eTable)
							return std::min(m_beginLen, m_endLen) + lookUp->queryMinDistance(srcPos, m_bTable);
						else
							return std::min(m_beginLen + lookUp->queryMinDistance(srcPos, m_bTable),
											m_endLen + lookUp->queryMinDistance(srcPos, m_eTable));
					}
					else
					{
						assert(m_bGridID != Tools::UINT32_MAX || m_eGridID != Tools::UINT32_MAX);
						if (m_beginID == m_endID)
						{
							assert(m_beginLen == 0 && m_endLen == 0);
							return lookUp->queryMinDistance(srcPos, m_bGridID);
						}
						else if (m_bGridID == m_eGridID)
							return std::min(m_beginLen, m_endLen) + lookUp->queryMinDistance(srcPos, m_bGridID);
						else
							return std::min(m_beginLen + lookUp->queryMinDistance(srcPos, m_bGridID),
											m_endLen + lookUp->queryMinDistance(srcPos, m_eGridID));
					}
				}
				double getMinDisToHere(const MapPos2D& p) const { return Tools::SpatialMath::getDistance(p, m_pos); }

				bool beSameNodeID(size_t id) const { return id == m_beginID || id == m_endID; }
				double getEndingDis(size_t nodeID) const { return nodeID == m_beginID ? m_beginLen : m_endLen; }

			private:
				MapPos2D m_pos;
				size_t m_beginID, m_endID;
				double m_beginLen, m_endLen;
				uint64_t m_roadID;

				size_t m_bGridID, m_eGridID;
				byte *m_bTable, *m_eTable;

			}; // DestInfo

			class VisitedNode
			{
			public:
				size_t m_nodeID, m_highID;
				double m_from, m_to;
				VisitedNode* m_prev;

			public:
				VisitedNode(size_t id, double from, const SingleLayerNW* network, const NVDataPublish::ILookUp* table, 
					const DestInfo& dest, VisitedNode* prev);
				VisitedNode(size_t lowID, size_t highID, double from, const SingleLayerNW* network, const NVDataPublish::ILookUp* table, 
					const DestInfo& dest, VisitedNode* prev);
				VisitedNode(size_t id, double from, VisitedNode* prev) 
					: m_nodeID(id), m_highID(0), m_from(from), m_to(0), m_prev(prev) {}
				VisitedNode(size_t id) 
					: m_nodeID(id), m_highID(0) {}

				void getShortestPath(std::vector<size_t>* pathNodeIDs) const;
				void getShortestPath(std::vector<std::pair<NODE_ID, NEAREST_DIS> >* path) const;

				struct LengthAscending : public std::binary_function<VisitedNode*, VisitedNode*, bool>
				{
					bool operator() (const VisitedNode* lhs, const VisitedNode* rhs) const {
						return lhs->m_from + lhs->m_to < rhs->m_from + rhs->m_to
							|| lhs->m_from + lhs->m_to == rhs->m_from + rhs->m_to && lhs->m_nodeID < rhs->m_nodeID;
					}
				};

				struct IDAscending : public std::binary_function<VisitedNode*, VisitedNode*, bool>
				{
					bool operator() (const VisitedNode* lhs, const VisitedNode* rhs) const {
						return lhs->m_nodeID < rhs->m_nodeID;
					}
				};

			private:
				void initToDis(const SingleLayerNW* network, const NVDataPublish::ILookUp* table,
					const DestInfo& dest);

			}; // VisitedNode

			typedef std::set<VisitedNode*, VisitedNode::IDAscending>::iterator PTR_VISITED;
			class VisitedList
			{
			public:
				~VisitedList() { clear(); }

				void push(VisitedNode* n) { m_data.insert(n); }

				PTR_VISITED find(size_t id) {
					VisitedNode n(id);
					return m_data.find(&n);
				}

				bool exist(PTR_VISITED ptr) const { return ptr != m_data.end(); }

				void clear() {
					PTR_VISITED itr = m_data.begin();
					for ( ; itr != m_data.end(); ++itr)
						delete *itr;
					m_data.clear();
				}

				// debug
				size_t size() const { return m_data.size(); }
				//

			private:
				std::set<VisitedNode*, VisitedNode::IDAscending> m_data;

			}; // VisitedList

			class CandidateList
			{
			public:
				bool empty() const { return m_data.empty(); }

				void push(VisitedNode* n) { m_data.insert(n); }

				VisitedNode* top() const { return *m_data.begin(); }

				void pop() { m_data.erase(m_data.begin()); }

				void update(VisitedNode* v, double len, VisitedNode* src) {
					std::set<VisitedNode*, VisitedNode::LengthAscending>::iterator itr
						= m_data.find(v);

					if (itr != m_data.end())
						m_data.erase(itr);

					v->m_from = len;
					v->m_prev = src;
					m_data.insert(v);
				}

				void clear() { m_data.clear(); }

			private:
				std::set<VisitedNode*, VisitedNode::LengthAscending> m_data;

			}; // CandidateList

			typedef SimulateNavigate::DisplayProcessInfo DisProInfo;

			class SearchingUtility
			{
			public:
				static void drawLine(const DisProInfo& info, uint64_t id);

				static void noSearchPath(const BindPoint& from, const BindPoint& to, std::vector<size_t>* routeNodeID);

			}; // SearchingUtility
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_SEARCHING_UTILITY_H_