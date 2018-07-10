#ifndef _IS_SDE_GENERALIZATION_ALGORITHM_STRUCTURAL_SIMPLIFICATION_H_
#define _IS_SDE_GENERALIZATION_ALGORITHM_STRUCTURAL_SIMPLIFICATION_H_
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
			class StructuralSimplification : public IAlgorithm 
			{
			public:
				StructuralSimplification(size_t k, size_t forbidDistance = 1);
				virtual ~StructuralSimplification();

				//
				// IOperator IAlgorithm
				//
				virtual void implement(const std::vector<IShape*>& src, std::vector<IShape*>& dest);
				virtual OperatorType type() const { return POINTS_SIMPLIFICATION; }

			private:
				class RandomSelective
				{
				public:
					~RandomSelective();

					void initSelectedData(const std::vector<MapPos2D>& pos, const std::vector<double>& impt);

					bool select(MapPos2D& pos);

				private:
					struct PosWithImportance
					{
						MapPos2D m_pos;
						double m_importance;
						double m_start;

						PosWithImportance(const MapPos2D& pos, double impt)
							: m_pos(pos), m_importance(impt), m_start(0) {}
						PosWithImportance(double start)
							: m_start(start) {}
					}; // PosWithImportance

					class PositionCmp
					{
					public:
						bool operator () (const PosWithImportance* lhs, const PosWithImportance* rhs) {
							return lhs->m_start < rhs->m_start;
						}
					}; // PositionCmp

				private:
					void clear();

					void reConstructStartPos();

				private:
					std::vector<PosWithImportance*> m_data;

				}; // RandomSelective

				class SelectStrategy
				{
				public:
					SelectStrategy(Algorithms::VoronoiByDelaunay* voronoi,
						size_t k, size_t forbidDistance);

					virtual void select(std::set<MapPos2D, PosCmp<double> >& points, std::vector<MapPos2D>& dest) const = 0;

				protected:
					void updateForbiddenSet(const MapPos2D& p, std::set<MapPos2D, PosCmp<double> >& forbidden) const;

				protected:
					Algorithms::VoronoiByDelaunay* m_voronoi;
					size_t m_k;
					size_t m_forbidDistance;

				}; // SelectStrategy

				class OrderSelectStrategy : public SelectStrategy
				{
				public:
					OrderSelectStrategy(Algorithms::VoronoiByDelaunay* voronoi,
						size_t k, size_t forbidDistance);

					virtual void select(std::set<MapPos2D, PosCmp<double> >& points, std::vector<MapPos2D>& dest) const;
				}; // OrderSelectStrategy

				class RandomSelectStrategy : public SelectStrategy
				{
				public:
					RandomSelectStrategy(Algorithms::VoronoiByDelaunay* voronoi,
						size_t k, size_t forbidDistance);

					virtual void select(std::set<MapPos2D, PosCmp<double> >& points, std::vector<MapPos2D>& dest) const;
				}; // RandomSelectStrategy

			private:
				void clear();

				void selective(const std::vector<MapPos2D>& src, std::vector<MapPos2D>& dest);
				void selectBoundLine(const std::vector<std::vector<MapPos2D> >& src, std::vector<MapPos2D>& dest) const;

			private:
				Algorithms::VoronoiByDelaunay* m_voronoi;

				std::set<MapPos2D, PosCmp<double> > m_points;

				size_t m_k;
				size_t m_forbidDistance;

			}; // StructuralSimplification
		}
	}
}

#endif // _IS_SDE_GENERALIZATION_ALGORITHM_STRUCTURAL_SIMPLIFICATION_H_