#ifndef _IS_SDE_NVDATAPUBLISH_PUBLISH_H_
#define _IS_SDE_NVDATAPUBLISH_PUBLISH_H_
#include "../Tools/SpatialDataBase.h"
#include <windows.h>

namespace IS_SDE
{
	namespace Render
	{
		class MapLayer;
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class Publish;
		}

		class Publish
		{
		public:
			static void publishNVData(
				HWND hwnd, 
				std::vector<Render::MapLayer*>& layers, 
				Tools::PropertySet& ps
				);
			// String                   Value		Description
			// --------------------------------------------------------------------------
			// [Arrangement]
			// MaxComputeRoadNum		VT_ULONG	The max number of roads to arrange each time.
			// Precision				VT_DOUBLE	IF the distance of one line's endpoint to another
			//										line less than it, the two lines will be 
			//										consider connected.
			// [Features]
			// PageSize					VT_ULONG	Specify the page size.
			// FilePath					VT_PWCHAR	The file path for result data.
			// PrecisionTransToInt		VT_ULONG	Double to int will reserve how many decimal.
			// CondenseStrategy			VT_ULONG	The method to condense geographic data.
			//
			// [Network]
			// Capacity					VT_ULONG	The buffer size for Network's Buffer.
			// WriteThrough				VT_BOOL
			// MaxComputeNodeNum		VT_ULONG	The max number of nodes to handle each time.
			//
			// [MultiLayersNW]
			// MaxNodeNumToCondense		VT_ULONG	The max number of nodes to condense per time.
			//
			// [LookUpTable]
			// MaxNodeNumPerLookUpTable	VT_ULONG	The max number of nodes in look up table's grid.

		private:
			static void createSingleLayerTopo(const Features::Publish& fp, Tools::PropertySet& ps);
			static void createMultiLayersTopo(HWND hwnd, Tools::PropertySet& ps);

			static void createLookUpTable(HWND hwnd, Tools::PropertySet& ps);

		}; // Publish
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_PUBLISH_H_
