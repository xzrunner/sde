#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_INTERVAL_TREE_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_INTERVAL_TREE_H_
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Edge;

			class IntervalTree
			{
				typedef std::multimap<double, Edge*> HOR_LIST;
				typedef std::pair<HOR_LIST::iterator, HOR_LIST::iterator> HOR_LIST_RANGE;

			public:
				void insert(Edge* e);
				void erase(Edge* e);

				void regionQuery(const MapScope& scope, std::vector<Edge*>* ret) const;

				//private:
				//	void regionQuery(const double& left, const double& right, std::vector<Edge*>* ret) const

			private:
				HOR_LIST m_mapLeftCoords;

			}; // IntervalTree

			//////////////////////////////////////////////////////////////////////////

			class SegmentTree
			{
			public:
				void insert(Edge* e);
				void erase(Edge* e);

				void regionQuery(const MapScope& scope, std::vector<Edge*>* ret);

			private:
				enum NodeType
				{
					LEFT	= 0,
					RIGHT	= 1
				};

				class Node
				{
				public:
					Node(const MapPos2D& pos, NodeType type, Edge* e) 
						: m_pos(pos), m_type(type), m_belongTo(e) {}

					Node(const MapPos2D& pos)
						: m_pos(pos), m_type(LEFT), m_belongTo(NULL) {}

				public:
					MapPos2D		m_pos;
					NodeType		m_type;
					Edge*			m_belongTo;
					std::set<Edge*>	m_through;

				}; // Node

				class NodeCmp
				{
				public:
					bool operator () (const Node& lhs, const Node& rhs) const;

				}; // NodeCmp

				typedef std::multiset<Node, NodeCmp>::iterator ITR_LIST;

			private:
				void findPosition(const Node& n, ITR_LIST& left, ITR_LIST& right);

				void getThroughEdges(ITR_LIST& p, std::set<Edge*>* result) const;

			private:
				std::multiset<Node, NodeCmp> m_list;

			}; // SegmentTree
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_INTERVAL_TREE_H_
