#ifndef _IS_SDE_SPATIALINDEX_VISITOR_H_
#define _IS_SDE_SPATIALINDEX_VISITOR_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../BasicType/BasicGMType.h"

namespace IS_SDE
{
	class GeoLine;

	namespace NVDataPublish
	{
		namespace Arrangement
		{
			class LineMgr;
		}
	}

	namespace SpatialIndex
	{
		// Count categories' number
		class CountCategoriesNumber
		{

		}; // CountCategoriesNumber

		// Fetch all objs.
		class FetchDataVisitor : public IVisitor
		{
		public:
			FetchDataVisitor(std::vector<IShape*>& shapes) 
				: m_shapes(shapes) {}

			bool shouldCheckEachData() const { return true; }
			void visitNode(const INode& n) {}
			void visitData(const IData& d, size_t dataIndex);
			void visitData(std::vector<const IData*>& v);
			void visitData(const INode& n) {}

		private:
			std::vector<IShape*>& m_shapes;

		}; // FetchDataVisitor

		// Fetch all objs and grid's MBR.
		class FetchDataAndGridMBRVisitor : public IVisitor
		{
		public:
			FetchDataAndGridMBRVisitor(std::vector<IShape*>& shapes, std::vector<IShape*>& bounds)
				: m_shapes(shapes), m_bounds(bounds) {}

			bool shouldCheckEachData() const { return true; }
			void visitNode(const INode& n);
			void visitData(const IData& d, size_t dataIndex);
			void visitData(std::vector<const IData*>& v);
			void visitData(const INode& n) {}

		private:
			std::vector<IShape*>& m_shapes, m_bounds;

		}; // FetchDataAndGridMBRVisitor

		// Fetch all obj's id.
		class QuerySelectionVisitor : public IVisitor
		{
			typedef std::vector<std::pair<id_type, std::vector<id_type> > > PAIR_NODEID_WITH_OBJID;

		public:
			//std::vector<std::pair<id_type, size_t> >	m_nodeIDWithObjSize;
			//std::vector<id_type>						m_visitedObjID;

			PAIR_NODEID_WITH_OBJID m_nodeIDWithObjID;

		public:
			bool shouldCheckEachData() const { return true; }
			void visitNode(const INode& n);
			void visitData(const IData& d, size_t dataIndex);
			void visitData(std::vector<const IData*>& v);
			void visitData(const INode& n) {}

			void clear();
			bool isEmpty() const;

		private:
			id_type m_nodeID;

		}; // QuerySelectionVisitor

		class GetGeoLineVisitor : public IVisitor
		{
		public:
			NVDataPublish::Arrangement::LineMgr* m_lineMgr;
			id_type m_nodeID;

		public:
			GetGeoLineVisitor(const double& precision, ISpatialIndex* index, size_t layerID, 
				NVDataPublish::Arrangement::LineMgr* lineMgr);

			bool shouldCheckEachData() const { return true; }
			void visitNode(const INode& n);
			void visitData(const IData& d, size_t dataIndex);
			void visitData(std::vector<const IData*>& v);
			void visitData(const INode& n) {}

		private:
			const double		m_precision;
			ISpatialIndex*		m_index;
			size_t				m_layerID;
			std::set<id_type>	m_setIDs;

		}; // GetGeoLineVisitor

		class GetSelectedGridsMBRVisitor : public IVisitor
		{
		public:
			Rect* m_mbr;

		public:
			GetSelectedGridsMBRVisitor(Rect* ret);

			bool shouldCheckEachData() const { return true; }
			void visitNode(const INode& n);
			void visitData(const IData& d, size_t dataIndex);
			void visitData(std::vector<const IData*>& v);
			void visitData(const INode& n) {}

		}; // GetSelectedGridsMBRVisitor

	}
}

#endif // _IS_SDE_SPATIALINDEX_VISITOR_H_

