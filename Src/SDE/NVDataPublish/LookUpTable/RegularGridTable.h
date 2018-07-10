#ifndef _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_REGULAR_GRID_TABLE_H_
#define _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_REGULAR_GRID_TABLE_H_
#include "../Base.h"
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace NVDataPublish
	{
		namespace LookUpTable
		{
			class RegularGridTable : public ILookUp
			{
			public:
				RegularGridTable(IStorageManager& storage);
				RegularGridTable(IStorageManager& storage, size_t nodeSize, 
					size_t capacity, const MapScope& scope);
				virtual ~RegularGridTable() {}

				virtual double queryMinDistance(const MapPos2D& sNodePos, size_t dGridID) const;

				virtual void loadTable(size_t destGridID, byte** data) const;
				virtual double queryMinDistance(const MapPos2D& src, const byte* table) const;

				virtual size_t getGridsCount() const { return m_gridNum; }
				virtual size_t getGridID(const MapPos2D& pos) const;
				virtual void outputIndexInfo(size_t& nextPage);

				virtual void drawTable(Render::GDIRender* render) const;

			public:
				static const size_t HEADER_PAGE		= 0;
				static const size_t DATA_START_PAGE = 1;

			private:
				void storeHeader();
				void loadHeader();

				void initTableIndex(size_t cNode, size_t capacity, const MapScope& scope);

			private:
				IStorageManager* m_buffer;

				size_t m_gridNum;

				MapScope m_mapScope;
				double m_edgeLength;
				size_t m_horGridSize;

			}; // RegularGridTable
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_LOOKUPTABLE_REGULAR_GRID_TABLE_H_
