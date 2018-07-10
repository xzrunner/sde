#ifndef _IS_SDE_GEO_SHAPE_H_
#define _IS_SDE_GEO_SHAPE_H_
#include "../Tools/SpatialDataBase.h"
#include "BasicGMType.h"

namespace IS_SDE
{
	class Line;

	class GeoShape
	{
	public:
		GeoShape(ISpatialIndex* index, size_t layerID, id_type nodeID, id_type objID, size_t objIndex);
		virtual ~GeoShape() {}

		// First is m_nodeID; Second is m_objIndex.
		void getStorePosition(std::pair<id_type, size_t>* pos) const;

		ISpatialIndex* getIndex() const;

		virtual IShape* getShape() const = 0;

		const std::wstring&	name() const { return m_name; }

	public:
		size_t			m_layerID;
		id_type			m_nodeID;
		id_type			m_objID;
		size_t			m_objIndex;

	protected:
		ISpatialIndex*	m_index;

		std::wstring	m_name;

	}; // GeoShape

	class GeoLine : public GeoShape
	{
	public:
		GeoLine(ISpatialIndex* index, Line* line, size_t layerID, id_type nodeID, id_type objID, size_t objIndex);
		virtual ~GeoLine();
		void getCoordinates(std::vector<MapPos2D>* coords) const;
		size_t posSize() const;
		void moveCtlPoi(size_t index, const MapPos2D& newPos);

		virtual IShape* getShape() const;

	private:
		Line*	m_line;

	}; // GeoLine
}

#endif // _IS_SDE_GEO_SHAPE_H_
