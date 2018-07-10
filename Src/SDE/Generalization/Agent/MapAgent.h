#ifndef _IS_SDE_GENERALIZATION_AGENT_MAP_AGENT_H_
#define _IS_SDE_GENERALIZATION_AGENT_MAP_AGENT_H_
#include "AbstractAgent.h"
#include "../../BasicType/Scope.h"

namespace IS_SDE
{
	namespace Generalization
	{
		namespace Agent
		{
			class GroupAgent;

			class MapAgent : public AbstractAgent
			{
			public:
				MapAgent(const std::vector<ISpatialIndex*>& layers);
				virtual ~MapAgent();

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
				// GeoScope					VT_PVOID	MBR. [MapScope*]
				// GeoShape					VT_PVOID	[std::vector<IShape*>*]
				virtual void proposePlans(std::vector<IAlgorithm*>& plans) const;
				virtual void triggerBestPlan(IAlgorithm* plan);
				virtual double calHappiness() const;

			public:
				static const size_t NUM_PER_GROUP;
				static const double MAP_EDGE_DIVIDE_THRESHOLD;

			private:
				// getAgentProperty
				size_t getCategoriesNumber() const;
				MapScope* getGeoScope() const;
				std::vector<IShape*>* getGeoShape() const;

				void clusterToGroups();
				void clusterPoints(const std::vector<IShape*>& shapes);

			private:
				std::vector<GroupAgent*> m_groups;

			}; // MapAgent
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_AGENT_MAP_AGENT_H_