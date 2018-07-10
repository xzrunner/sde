#ifndef _IS_SDE_GENERALIZATION_AGENT_POINT_AGENT_H_
#define _IS_SDE_GENERALIZATION_AGENT_POINT_AGENT_H_
#include "AbstractAgent.h"

namespace IS_SDE
{
	class Point;

	namespace Generalization
	{
		namespace Agent
		{
			class PropertyBuffer;

			class PointAgent : public AbstractAgent
			{
			public:
				PointAgent(IShape& s);
				virtual ~PointAgent();

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
				// Coordinates				VT_PVOID	Points' Coordinates. [std::vector<MapPos2D>*]
				// GeoShape					VT_PVOID	[std::vector<IShape*>*]

			private:
				virtual void storeToShapes(std::vector<IShape*>& shapes) const;
				virtual void loadFromShapes(const std::vector<IShape*>& shapes);

			private:
				Point* m_point;

			}; // PointAgent
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_POINT_AGENT_H_