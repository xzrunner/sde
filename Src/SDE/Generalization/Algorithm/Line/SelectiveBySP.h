#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_SELECTIVE_BY_SP_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_SELECTIVE_BY_SP_H_
#include "../../GeneralizationBase.h"
#include "../../../SimulateNavigate/NavigateBase.h"

namespace IS_SDE
{
	class Line;

	namespace Render
	{
		class MapLayer;
	}

	namespace SimulateNavigate
	{
		namespace Network
		{
			class SingleLayerNW;
		}
	}

	namespace Generalization
	{
		namespace Algorithm
		{
			class SelectiveBySP : public IAlgorithm
			{
			public:
				SelectiveBySP(const std::wstring& topoFilePath, size_t divideNum);
				virtual ~SelectiveBySP();

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return LINES_SELECTIVE; }

				// todo: temporary, should get these shapes by SP
				// SP will add an interface with MapPos input.
				void getBorderIntersectShapes(std::vector<IShape*>& shapes) const;

			private:
				void initLayers(IStorageManager* sm, const std::wstring& path);

				void computeSPRoutes();
				void calSetsIntersection(size_t g0, size_t g1, size_t g2, size_t g3);

			private:
				size_t m_nDivide;

				std::vector<Render::MapLayer*> m_layers;
				std::vector<NVDataPublish::STATIC_LAYER*> m_indexes;

				SimulateNavigate::IPointBindLine* m_bind;
				IStorageManager* m_file;
				StorageManager::IBuffer* m_buffer;
				SimulateNavigate::Network::SingleLayerNW* m_network;

				std::vector<std::vector<uint64_t> > m_calBuffer;
				std::vector<uint64_t> m_destIDs;

			}; // SelectiveBySP
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_SELECTIVE_BY_SP_H_