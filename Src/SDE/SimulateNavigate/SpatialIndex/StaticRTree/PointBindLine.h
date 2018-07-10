#ifndef _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_POINT_BIND_LINE_H_
#define _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_POINT_BIND_LINE_H_
#include "../../NavigateBase.h"
#include "../../../NVDataPublish/Base.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace SpatialIndex
		{
			namespace StaticRTree
			{
				class PointBindLine : public IPointBindLine
				{
				public:
					PointBindLine(
						const std::vector<ISpatialIndex*>& layers,
						const NVDataPublish::ICondenseStrategy* cs
						);

					virtual void bind(const MapPos2D& p, Line** line, uint64_t* id) const;

				private:
					class NNEntry
					{
					public:
						id_type m_id;
						IEntry* m_pEntry;
						double m_minDist;

						NNEntry(id_type id, IEntry* e, double f) : m_id(id), m_pEntry(e), m_minDist(f) {}
						~NNEntry() {}

						struct ascending : public std::binary_function<NNEntry*, NNEntry*, bool>
						{
							bool operator()(const NNEntry* __x, const NNEntry* __y) const { return __x->m_minDist > __y->m_minDist; }
						};
					}; // NNEntry

					class NNComparator : public INearestNeighborComparator
					{
					public:
						double getMinimumDistance(const IShape& query, const IShape& entry)
						{
							return query.getMinimumDistance(entry);
						}

						double getMinimumDistance(const IShape& query, const IData& data)
						{
							IShape* pS;
							data.getShape(&pS);
							double ret = query.getMinimumDistance(*pS);
							delete pS;
							return ret;
						}
					}; // NNComparator

				private:
					std::vector<StaticRTree*> m_layers;
					const NVDataPublish::ICondenseStrategy* m_condenseStrategy;

				}; // PointBindLine
			}
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_POINT_BIND_LINE_H_