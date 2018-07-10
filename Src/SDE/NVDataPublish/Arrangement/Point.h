#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_POINT_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_POINT_H_
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Edge;

			class Node
			{
			public:
				Node() : m_prev(NULL), m_next(NULL), m_id(0) {}

				const MapPos2D& pos() const { return m_pos; }

				void addEdge(Edge* e, bool isBegin);
				void eraseEdge(Edge* e, bool isBegin);

				void getAllEdges(std::vector<Edge*>* all) const;
				void getUpAndLowEdges(std::vector<Edge*>* upIsP, std::vector<Edge*>* lowIsP) const;

				bool merge(Node* n);

				bool isSingle() const { return m_begin.size() + m_end.size() == 0; }

			private:
				void setDirtyState();

				void setType(const Rect& region, bool considerGeoAttr = true);

			private:
				enum NodeType
				{
					END			= 0,
					COMMON		= 1,
					INTERSECT	= 2
				};

			private:
				// For Adjacent List
				id_type m_id;

			private:
				std::set<Edge*>	m_begin, m_end;
				MapPos2D		m_pos;

				// It is a ring after using a precision to merge the node's nearby nodes.
				Node			*m_prev, *m_next;

				NodeType		m_type;

				friend class NodeMgr;
				friend class Edge;
				friend class PolyEdge;

				friend class ALGrid;

			}; // Node

			// All ComputedGrids will use the only NodeMgr.
			// Each ComputedGrids: before use it should call init(), finished should call clear();
			// After all ComputedGrids finished, ~NodeMgr() will delete the pool.
			// Maybe it is better to use Singleton, but how to delete the Singleton (delete pool) ?
			class NodeMgr
			{
				typedef std::map<id_type, std::vector<Node*> > MAP_GRID_TO_NODES;

			public:
				~NodeMgr();

				void init(const Rect& computeRegion, const double& precision);
				void clear(bool usePool = false);

				// Return true: create a new node.
				// Return false: same with an exist node.
				bool createNode(const MapPos2D& p, Node** n);

				const MAP_GRID_TO_NODES& getAllData() const { return m_grid.m_mapGridToNodes; }

				void setAllNodesType(const Rect& region, bool considerGeoAttr = true);

				void queryTopoNodesByRect(const MapScope& r, std::vector<Node*>* nodes) const;

			private:
				class RegularGrid
				{
				public:
					void init(const Rect& computeRegion, const double& precision);

					bool insertNode(const MapPos2D& p, Node** n, std::stack<Node*>* pool);

					void recycle(std::stack<Node*>&	pool, bool usePool = false);

					void queryTopoNodesByRect(const MapScope& r, std::vector<Node*>* nodes) const;

				private:
					// Get the position of p. Return grid ID = xPos * yGridSize + yPos
					id_type getGridID(const MapPos2D& p) const;

					// Get the 9 grids' IDs near the center ID.
					void getNearbyPos(const id_type& id, std::vector<id_type>* nearbyGrids) const;

				private:
					class LayerInfo
					{
					public:
						void init(const Rect& computeRegion, const double& precision);

					public:
						double		gridEdgeLength;		// Also it is the min distance of 2 point.
						MapPos2D	leftLowPos;
						MapPos2D	rightTopPos;
						size_t		xGridSize;
						size_t		yGridSize;

					}; // LayerInfo

				private:
					LayerInfo			m_layerInfo;
					MAP_GRID_TO_NODES	m_mapGridToNodes;

					friend class NodeMgr;

				}; // RegularGrid


			private:
				RegularGrid			m_grid;
				std::stack<Node*>	m_pool;

			}; // NodeMgr
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_POINT_H_
