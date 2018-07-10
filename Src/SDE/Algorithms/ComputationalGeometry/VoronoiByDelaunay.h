#ifndef _IS_SDE_ALGORITHMS_VORONOI_BY_DELAUNAY_H_
#define _IS_SDE_ALGORITHMS_VORONOI_BY_DELAUNAY_H_
#include "DelaunayTriangulation.h"

namespace IS_SDE
{
	class IShape;

	namespace Algorithms
	{
		class VoronoiByDelaunay
		{
		public:
			VoronoiByDelaunay(const std::vector<MapPos2D>& src);
			~VoronoiByDelaunay();

			void getAllNodePos(std::vector<MapPos2D>& pos) const;

			void getVoronoiNodeBoundLine(std::vector<IShape*>& shapes) const;

			double getNodeExtendArea(const MapPos2D& n) const;

			void getSurroundPos(const MapPos2D& pos, std::vector<MapPos2D>& surround) const {
				m_dt.getSurroundPos(pos, surround);
			}

			bool isBoundNode(const MapPos2D& n) const {
				return m_dt.isBoundNode(n);
			}
			void getBoundLinePos(std::vector<std::vector<MapPos2D> >& bounds) const {
				return m_dt.getBoundLinePos(bounds);
			}

			bool insertNode(const MapPos2D& p);
			void deleteNodes(const std::vector<MapPos2D>& pos) {
				m_dt.deleteNodes(pos);
				initNodes();
			}

		private:
			typedef DelaunayTriangulation::Node		D_NODE;
			typedef DelaunayTriangulation::NodeCmp	D_NODE_CMP;
			typedef DelaunayTriangulation::Triangle	D_TRI;

			class Node
			{
			public:
				Node(D_NODE* n);
				Node(const MapPos2D& p) : m_center(p), m_shapeLine(NULL) {}
				~Node();

			private:
				MapPos2D m_center;
				std::vector<MapPos2D> m_bound;
				IShape* m_shapeLine;
				double m_area;

				friend class VoronoiByDelaunay;

			}; // Node

			class NodeCmp
			{
			public:
				bool operator () (const Node* lhs, const Node* rhs) const {
					return lhs->m_center.d0 < rhs->m_center.d0 
						|| lhs->m_center.d0 == rhs->m_center.d0 && lhs->m_center.d1 < rhs->m_center.d1;
				}
			}; // NodeCmp

		private:
			void initNodes();

		private:
			DelaunayTriangulation m_dt;

			std::set<Node*, NodeCmp> m_nodes;

		}; // VoronoiByDelaunay
	}
}

#endif // _IS_SDE_ALGORITHMS_VORONOI_BY_DELAUNAY_H_