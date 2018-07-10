#ifndef _IS_SDE_GENERALIZATION_AGENT_TOOLBOX_H_
#define _IS_SDE_GENERALIZATION_AGENT_TOOLBOX_H_
#include "../../Tools/SpatialDataBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Agent
		{
			enum ToolType
			{
				TT_DELAUNAY_TRIANGULATION,	// Algorithms::DelaunayTriangulation*
				TT_VORONOI,					// Algorithms::VoronoiByDelaunay*
				TT_SPATIAL_INDEX,			// ISpatialIndex*
				TT_SPATIAL_INDEXES			// std::vector<ISpatialIndex*>*

			}; // ToolType

			class Toolbox
			{
			public:
				~Toolbox();

				void addTool(ToolType type, void* tool);
				void* fetchTool(ToolType type) const;

			private:
				void deleteTool(ToolType type, void* tool);

			private:
				std::map<ToolType, void*> m_tools;

			}; // Toolbox
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_TOOLBOX_H_
