#ifndef _IS_SDE_NVDATAPUBLISH_BASE_H_
#define _IS_SDE_NVDATAPUBLISH_BASE_H_
#include "../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Render
	{
		class GDIRender;
	}

	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class StaticRTree;
			}
		}
	}

	namespace NVDataPublish
	{
		typedef SimulateNavigate::SpatialIndex::StaticRTree::StaticRTree STATIC_LAYER;

		enum CondenseStrategyType
		{
			CS_NO_OFFSET,
			CS_CERTAINTY_OFFSET,
			CS_UNCERTAINTY_OFFSET
		};

		//
		// Interfaces
		//

		interface ICondenseStrategy
		{
		public:
			// Only the IShape data size, not including the Node info. 
			virtual size_t getType() const = 0;
			virtual size_t allShapeDataSize(const INode* n) const = 0;
			virtual size_t dataSize(const IShape* s) const = 0;
			virtual void loadFromByteArray(IShape** s, const byte* data, size_t length) const = 0;
			virtual void storeToByteArray(const IShape* s, byte** data, size_t& length) const = 0;

			virtual ~ICondenseStrategy() {}
		}; // ICondenseStrategy

		interface ILookUp
		{
		public:
			virtual double queryMinDistance(const MapPos2D& sNodePos, size_t dGridID) const = 0;

			virtual void loadTable(size_t destGridID, byte** data) const = 0;
			virtual double queryMinDistance(const MapPos2D& src, const byte* table) const = 0;

			// for TableBuilding
			virtual size_t getGridsCount() const = 0;
			virtual size_t getGridID(const MapPos2D& pos) const = 0;
			virtual void outputIndexInfo(size_t& nextPage) = 0;

			// for test
			virtual void drawTable(Render::GDIRender* render) const = 0;

			virtual ~ILookUp() {}
		}; // ILookUp
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_BASE_H_
