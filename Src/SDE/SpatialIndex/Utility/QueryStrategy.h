#ifndef _IS_SDE_SPATIALINDEX_QUERY_STRATEGY_H_
#define _IS_SDE_SPATIALINDEX_QUERY_STRATEGY_H_
#include "../../Tools/SpatialDataBase.h"
#include "../../BasicType/Rect.h"

namespace IS_SDE
{
	class Point3D;

	namespace SpatialIndex
	{
		class TraverseByLevelQueryStrategy : public IQueryStrategy
		{
		private:
			std::queue<id_type> m_ids;

		public:
			void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext);
			bool shouldStoreEntry() { return false; }

		}; // TraverseByLevelQueryStrategy

		class LayerRegionQueryStrategy : public IQueryStrategy
		{
		public:
			Rect m_indexedSpace;

		public:
			void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext);
			bool shouldStoreEntry() { return false; }

		}; // LayerRegionQueryStrategy

		// Get all Point3D in the layer
		// Be used at draw contour line task.
		class GetAllPoint3DQueryStrategy : public IQueryStrategy
		{
		public:
			std::vector<Point3D*> m_points;

		private:
			std::queue<id_type> m_ids;

		public:
			void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext);
			bool shouldStoreEntry() { return false; }

		}; // GetAllPoint3DQueryStrategy
	}
}

#endif // _IS_SDE_SPATIALINDEX_QUERY_STRATEGY_H_