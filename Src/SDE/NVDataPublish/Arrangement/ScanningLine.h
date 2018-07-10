#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_SCANNING_LINE_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_SCANNING_LINE_H_
#include "../../Tools/Tools.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Edge;

			class ScanningLine
			{
			public:
				ScanningLine(const double& precision);

				void insert(Edge* e);
				void insert(Edge* e, const double& scanLine);
				void erase(Edge* e, const double& scanLine, const double& precision);

				void regionQuery(const MapPos2D& left, const MapPos2D& right, std::vector<Edge*>* ret) const;

				void findNeighbor(const MapPos2D& p, Edge** left, Edge** right) const;
				void findBoundary(const std::vector<Edge*>& up, Edge** left, Edge** right) const;

			private:
				//////////////////////////////////////////////////////////////////////////
				class SegCmp;

				class Segment
				{
				public:
					// Initial insert Segment
					Segment(Edge* e);

					// After node break in Segment
					Segment(Edge* e, const double& scanLine);

					// Used for region query
					Segment(const MapPos2D& p);

				private:
					MapPos2D	m_compPos;
					Edge*		m_edge;
					const bool	m_bPoint;

					friend class SegCmp;
					friend class ScanningLine;

				}; // Segment

				//////////////////////////////////////////////////////////////////////////
				class SegCmp
				{
				public:
					SegCmp(const double& precision);

					bool operator () (const Segment& lhs, const Segment& rhs) const;

				private:
					// return: equal is 0. less is -1. more is 1.
					static int compPointWithLine(const MapPos2D& p, const Edge& l, const double& precision);

					// return: equal is 0. less is -1. more is 1.
					static int compLineWithLine(const Segment& lhs, const Segment& rhs, const double& precision);

				private:
					const double m_precision;

					friend class ScanningLine;

				}; // SegCmp

				//////////////////////////////////////////////////////////////////////////

			private:
				typedef std::multiset<Segment, SegCmp> SCANNING_LIST;
				typedef std::pair<SCANNING_LIST::iterator, SCANNING_LIST::iterator> SCANNING_LIST_RANGE;

			private:
				SCANNING_LIST::iterator regionQuery(const MapPos2D& left, const MapPos2D& right, const Edge* e);

			private:
				const double m_precision;
				SCANNING_LIST m_list;

			}; // ScanningLine
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_SCANNING_LINE_H_
