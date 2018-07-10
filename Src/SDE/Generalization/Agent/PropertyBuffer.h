#ifndef _IS_SDE_GENERALIZATION_AGENT_PROPERTY_BUFFER_H_
#define _IS_SDE_GENERALIZATION_AGENT_PROPERTY_BUFFER_H_
#include "../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Agent
		{
			class PropertyBuffer
			{
			public:
				PropertyBuffer(IAgent* agent);
				~PropertyBuffer();
				
				void clearBuffer();

				void loadProperty(const std::string& property, Tools::Variant& var) const;
				// Property                 Value		Description
				// ---------------------------------------------------------------------------------------------
				// CategoriesNumber			VT_ULONG	The number of shapes.
				// GeoScope					VT_PVOID	MBR. [MapScope*]
				// GeoShape					VT_PVOID	[std::vector<IShape*>*]
				// Coordinates				VT_PVOID	Points' Coordinates. [std::vector<MapPos2D>*]
				// MinimalDistance			VT_DOUBLE	Minimum distance between objects.

				void storeProperty(const std::string& property, Tools::Variant& var);

			private:
				void deleteProperty(const std::string& property, Tools::Variant& var) const;

			private:
				IAgent* m_agent;

				std::map<std::string, Tools::Variant> m_pool;

			}; // PropertyPool
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_PROPERTY_BUFFER_H_