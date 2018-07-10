#ifndef _IS_SDE_GENERALIZATION_AGENT_POINTS_AGENT_H_
#define _IS_SDE_GENERALIZATION_AGENT_POINTS_AGENT_H_
#include "GroupAgent.h"
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	class IShape;

	namespace Generalization
	{
		namespace Agent
		{
			class PointAgent;

			class PointsAgent : public GroupAgent
			{
			public:
				PointsAgent(const std::vector<IShape*>& src);
				virtual ~PointsAgent();

				//
				// ISerializable interface
				//
				virtual size_t getByteArraySize() const;
				virtual void loadFromByteArray(const byte* data);
				virtual void storeToByteArray(byte** data, size_t& length) const;

				//
				// IAgent interface
				//
				virtual void getAgentProperty(const std::string& property, Tools::Variant& var) const;
				// Property                 Value		Description
				// ---------------------------------------------------------------------------------------------
				// CategoriesNumber			VT_ULONG	The number of shapes.
				// Coordinates				VT_PVOID	Points' Coordinates. [std::vector<MapPos2D>*]
				// GeoScope					VT_PVOID	MBR. [MapScope*]
				// MinimalDistance			VT_DOUBLE	Minimum distance between objects.
				// NearestTwoPoints			VT_PVOID	Two points' coords. [MapPos2D[2]].
				// GeoShape					VT_PVOID	[std::vector<IShape*>*]
				// SortedVoronoiArea		VT_PVOID	The area list of all points. [(size_t + std::vector<double>)*]
   
			private:
				virtual void storeToShapes(std::vector<IShape*>& shapes) const;
				virtual void loadFromShapes(const std::vector<IShape*>& shapes);

				void addDelaunayTriangulationTool(const std::vector<MapPos2D>& src);
				void addVoronoiTool(const std::vector<MapPos2D>& src);

				// For getAgentProperty
				std::vector<MapPos2D>* getCoordinates() const;
				MapScope* getGeoScope() const;
				double getMinimalDistance(MapPos2D* nearestPos) const;
				std::vector<IShape*>* getGeoShape() const;
				byte* getSortedVoronoiArea() const;

			private:
				std::vector<PointAgent*> m_points;

			}; // PointsAgent
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_POINTS_AGENT_H_