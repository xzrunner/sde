#ifndef _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_LINE_H_
#define _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_LINE_H_
#include "../../BasicType/GeoShape.h"
#include "../../SpatialIndex/NVRTree/NVRTree.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class Node;
			class PolyEdge;

			class Edge
			{
			public:
				void init(Node* pBNode, Node* pENode, PolyEdge* pPolyEdge, const double& precision);
				void clear();

				const MapPos2D& beginPos() const;
				const MapPos2D& endPos() const;

				Node* beginNode() const { return m_begin; }
				Node* endNode() const { return m_end; }

				bool isHorizontal() const { return m_scanInfo.slope == Tools::DOUBLE_MAX; }
				bool isBeginHigher() const { return m_scanInfo.bStartHigher; }

				double getSlope() const { return m_scanInfo.slope; }
				void updateSlope() { m_scanInfo.update(beginPos(), endPos()); }

				Edge* cut(Node* n, const double& precision);

				void setDirtyState();

				bool shouldEraseDuplicated() const;

				bool isSameAttr(const Edge* e) const;

			private:
				void addEdgeToNode();

				bool isLowerGradeThan(const Edge& e) const;

			private:
				class ScanInfo
				{
				public:
					bool bStartHigher;

					// - dx/dy 
					// Used for sorting at Scanning
					double slope;

				public:
					//	ScanInfo() {}
					//	ScanInfo(const ScanInfo& info) : bStartHigher(info.bStartHigher), slope(info.slope) {}

					void init(const MapPos2D& begin, const MapPos2D& end, const double& precision);
					void update(const MapPos2D& begin, const MapPos2D& end);

				};

			private:
				Node		*m_begin, *m_end;
				Edge		*m_prev, *m_next;
				PolyEdge*	m_pPolyEdge;
				ScanInfo	m_scanInfo;

				friend class Node;
				friend class PolyEdge;

			}; // Edge

			class NodeMgr;

			class PolyEdge
			{
			public:
				bool init(GeoLine* pGeoLine, NodeMgr* pNodeMgr, std::stack<Edge*>* edgePool, const double& precision);
				void clear(std::stack<PolyEdge*>& polyEdgePool, std::stack<Edge*>& edgePool, bool usePool = false);
				void clear();

				void eraseDuplicate(std::stack<PolyEdge*>* polyEdgePool);

				void breakIn(std::stack<PolyEdge*>* polyEdgePool);

				void connect();

				// The bool will be set true when the Line* extended.
				void transToLine(std::vector<std::pair<IShape*, bool> >* newData, ISpatialIndex* layer, LineMgr* lineMgr);

			private:
				void updateEdgePointToPolyEdge();

				void connectOneDirection(PolyEdge* pe, bool bConnectNext, PolyEdge* original) const;

				void getConnectPos(const Edge* e, bool beginToEnd, std::vector<MapPos2D>* result,
					std::vector<PolyEdge*>* connectedPE) const;

				void setCountEdge();

				void createGeoLine(const std::vector<MapPos2D>& pos, ISpatialIndex* layers,
					GeoLine** geoLine) const;

			private:
				enum PEType
				{
					COMMON		= 0,
					ERASE		= 1,
					CONNECTED	= 2
				};

			private:
				GeoLine*	m_pGeoLine;
				Edge*		m_begin;
				bool		m_bDirty;

				// Erase the duplicate parts.
				PolyEdge*	m_next;

				PEType		m_type;

				size_t		m_countEdge;

				friend class Edge;
				friend class LineMgr;

			}; // PolyEdge

			class LineMgr
			{
			public:
				~LineMgr();

				void init(const double& precision, const Rect& computeRegion, 
					const std::vector<ISpatialIndex*>& layers, NodeMgr* pNodeMgr);
				void clear(bool usePool = false);

				void insertGeoLine(GeoLine* geoLine);
				void insertGeoLine(GeoLine* geoLine, const double& precision);

				void eraseDuplicateRoads();

				void breakInRoads();

				void connectRoads();

				void updateToDatabase(const std::vector<ISpatialIndex*>& layers);

			private:
				class ModifiedLine
				{
				public:
					ModifiedLine(PolyEdge* line, ISpatialIndex* index, LineMgr* lineMgr, size_t lineIndex);
					void updateToDatabase(SpatialIndex::NVRTree::NodePtr n);
					bool empty() const { return m_newData.empty(); }

				private:
					size_t m_oldIndexInGrid;
					std::vector<std::pair<IShape*, bool> > m_newData;

					friend class LineMgr;

				}; // ModifiedLine

			private:
				NodeMgr* m_pNodeMgr;

				std::vector<GeoLine*>	m_pGeoLine;

				std::vector<PolyEdge*>	m_pPoylyEdges;
				std::stack<PolyEdge*>	m_polyEdgePool;

				std::stack<Edge*>		m_edgePool;

			}; // LineMgr
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_ARRANGEMENT_LINE_H_
