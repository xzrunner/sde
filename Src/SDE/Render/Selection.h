#ifndef _IS_SDE_RENDER_SELECTION_H_
#define _IS_SDE_RENDER_SELECTION_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace SpatialIndex
	{
		class QuerySelectionVisitor;
	}

	namespace Render
	{
		class MapLayer;
		class Selection
		{
			class SelectionObj;
			class SelectionObjComp;

		public:
			typedef std::map<MapLayer*, std::set<SelectionObj, SelectionObjComp> > MAP_LAYER_TO_OBJS;
			typedef std::set<SelectionObj, SelectionObjComp> SET_ALL_OBJS_IN_LAYER;

		public:
			void insert(MapLayer* mapLayer, id_type nodeID, id_type objID);
			void insert(MapLayer* mapLayer, const SpatialIndex::QuerySelectionVisitor& vis);

			void deleteLayer(MapLayer* mapLayer);
			void clear() { m_selection.clear(); }
			bool isEmpty() const { return m_selection.empty(); }

			void getAllShapes(std::vector<IShape*>& shapes) const;
			void getAllShapes(std::vector<IShape*>& shapes, std::vector<ISpatialIndex*>& indexes,
				std::vector<id_type>& objIDs) const;

		private:
			class SelectionObj
			{
			public:
				SelectionObj(id_type nodeID, id_type objID);

				id_type m_nodeID;
				id_type m_objID;
			}; // SelectionObj

			class SelectionObjComp
			{
			public:
				bool operator() (const SelectionObj& lhs, const SelectionObj& rhs) const;
			}; // SelectionObjComp

		private:
			MAP_LAYER_TO_OBJS m_selection;

		}; // Selection
	}
}

#endif // _IS_SDE_RENDER_SELECTION_H_
