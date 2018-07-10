#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_SETTLEMENT_SPACING_RATIO_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_SETTLEMENT_SPACING_RATIO_H_
#include "../../GeneralizationBase.h"

namespace IS_SDE
{
	namespace Algorithms
	{
		class VoronoiByDelaunay;
	}

	namespace Generalization
	{
		namespace Algorithm
		{
			class QTree;

			class SettlementSpacingRatio : public IAlgorithm
			{
			public:
				SettlementSpacingRatio(size_t k) 
					: m_k(k), m_voronoi(NULL), m_index(NULL), m_constant(1.0) {}
				virtual ~SettlementSpacingRatio();

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return POINTS_SELECTIVE; }

			private:
				struct SignedPoint
				{
					MapPos2D m_pos;
					size_t m_index;
					bool m_bTested;
					double m_importance;

					SignedPoint(const MapPos2D& p) : m_pos(p) {}
					SignedPoint(const MapPos2D& p, size_t i, double importance) 
						: m_pos(p), m_index(i), m_bTested(false), m_importance(importance) {}

				}; // SignedPoint

				class PosCmp
				{
				public:
					bool operator () (const SignedPoint* lhs, const SignedPoint*  rhs) const {
						return lhs->m_pos.d0 < rhs->m_pos.d0
							|| lhs->m_pos.d0 == rhs->m_pos.d0 && lhs->m_pos.d1 < rhs->m_pos.d1;
					}
				}; // PosCmp

				class ImportanceCmp
				{
				public:
					bool operator () (const SignedPoint* lhs, const SignedPoint*  rhs) const {
						return lhs->m_importance > rhs->m_importance;
					}

				}; // ImportanceCmp

			private:
				void clear();

				void selective(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest);
				void selectBoundLine(const std::vector<std::vector<MapPos2D> >& src, std::vector<MapPos2D>& dest) const;
				bool isShouldSelected(const SignedPoint& p) const;

			private:
				static const size_t INDEX_CAPACITY = 10;

			private:
				Algorithms::VoronoiByDelaunay* m_voronoi;

				// todo: should move to class Agent
				QTree* m_index;

				size_t m_k;

				std::set<SignedPoint*, PosCmp> m_points;

				double m_constant;

			}; // SettlementSpacingRatio
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_SETTLEMENT_SPACING_RATIO_H_
