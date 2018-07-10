#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ROADS_BREAK_IN_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ROADS_BREAK_IN_H_
#include "IntervalTree.h"
#include "ScanningLine.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class NodeMgr;
			class Node;
			class Edge;

			class RoadsBreakIn
			{
				class EventPoint;

			public:
				RoadsBreakIn(NodeMgr* nodeMgr, const double& precision);

				void handleEventList(double* progress);

			private:
				void handleEventPoint(const EventPoint& p);

			private:

				//////////////////////////////////////////////////////////////////////////
				// EventList
				//////////////////////////////////////////////////////////////////////////

				enum EventPointType
				{
					UP		= 0,
					CONTAIN	= 1,
					DOWN	= 2
				};

				class EventPointComp;
				class EventList;

				class EventPoint
				{
				public:
					EventPoint(Node* n, EventPointType type, const double& precision);
					EventPoint(Node* n, EventPointType type, const MapPos2D& pos);

					double scanningLine() const;

					bool isIntersect() const { return m_node == NULL; }

				private:
					Node*			m_node;
					MapPos2D		m_pos;
					EventPointType	m_type;

					friend class EventPointComp;
					friend class EventList;
					friend class RoadsBreakIn;

				}; // EventPoint

				//////////////////////////////////////////////////////////////////////////

				class EventPointComp
				{
				public:
					bool operator () (const EventPoint& lhs, const EventPoint& rhs) const;

				}; // EventPointComp

				//////////////////////////////////////////////////////////////////////////

				class EventList
				{
					typedef std::set<EventPoint, EventPointComp> EVENT_LIST;

				public:
					EventList(const NodeMgr* pNodeMgr, const double& precision);

					double high() const;
					double low() const;

					bool empty() const { return m_list.empty(); }
					EventPoint top() { return *m_list.begin(); }
					void pop() { m_list.erase(m_list.begin()); }

					void insert(Node* n);
					void insert(Node* n, const MapPos2D& pos);
					void insertCross(const MapPos2D& pos);
					void insertHor(const MapPos2D& pos);
					void insertDownEvent(Node* n);

				private:
					const double	m_precision;
					EVENT_LIST		m_list;

				}; // EventList

				//////////////////////////////////////////////////////////////////////////
				// StateList
				//////////////////////////////////////////////////////////////////////////

				class StateList
				{
				public:
					StateList(const double& precision);

					void adjustOrder(const MapPos2D& p, const double& precision);

					void updateUpEvent(const std::vector<Edge*>& upIsP, const std::vector<Edge*>& lowIsP, const double& scanLine);
					void updateDownEvent(const std::vector<Edge*>& lowIsP);

					void checkNewEventNode(Node* n, EventList* pEventList);
					void checkNewEventNode(const MapPos2D& p, EventList* pEventList);

					void nodeBreakIn(Node* n, const MapPos2D& real, EventList* pEventList);

				private:
					void handleTwoSegsIntersect(Edge* lhs, Edge* rhs, const MapPos2D& currEvent, 
						EventList* pEventList);

					void nodeBreakIn(Node* n, Edge* e, const MapPos2D& real);

				private:
					const double	m_precision;

					ScanningLine	m_down;
					IntervalTree	m_tot;
					//SegmentTree		m_tot2;

				}; // StateList


			private:
				const double	m_precision;
				NodeMgr*		m_pNodeMgr;

				EventList		m_eventList;
				StateList		m_stateList;

			}; // RoadsBreakIn
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_ROADS_BREAK_IN_H_