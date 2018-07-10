#ifndef _IS_SDE_SCOPE_H_
#define _IS_SDE_SCOPE_H_
#include "Rect.h"

namespace IS_SDE
{
	template<class T>
	class Scope
	{
	public:
		Scope() : m_xMin(std::numeric_limits<T>::max()), m_xMax(std::numeric_limits<T>::min()), 
			m_yMin(m_xMin), m_yMax(m_xMax) {}

		Scope(T left, T top, T right, T bottom)
			: m_xMin(left), m_xMax(right), m_yMax(top), m_yMin(bottom) {}
		Scope(const Rect& r)
			: m_xMin(r.m_pLow[0]), m_xMax(r.m_pHigh[0]), m_yMin(r.m_pLow[1]), m_yMax(r.m_pHigh[1]) {}
		Scope(const Pos2D<T>& lhs, const Pos2D<T>& rhs)
		{
			if (lhs.d0 < rhs.d0)
			{
				m_xMin = lhs.d0;
				m_xMax = rhs.d0;
			}
			else
			{
				m_xMin = rhs.d0;
				m_xMax = lhs.d0;
			}

			if (lhs.d1 < rhs.d1)
			{
				m_yMin = lhs.d1;
				m_yMax = rhs.d1;
			}
			else
			{
				m_yMin = rhs.d1;
				m_yMax = lhs.d1;
			}
		}

		const Scope& operator = (const Rect& r) {
			m_xMin = r.m_pLow[0]; m_xMax = r.m_pHigh[0];
			m_yMin = r.m_pLow[1]; m_yMax = r.m_pHigh[1]; 
			return *this;
		}

		const Scope& operator = (const Scope& s) {
			m_xMin = s.m_xMin; m_xMax = s.m_xMax;
			m_yMin = s.m_yMin; m_yMax = s.m_yMax; 
			return *this;
		}

		void merge(const Scope<T>& scope) {
			m_xMin = std::min(scope.m_xMin, m_xMin);
			m_yMin = std::min(scope.m_yMin, m_yMin);
			m_xMax = std::max(scope.m_xMax, m_xMax);
			m_yMax = std::max(scope.m_yMax, m_yMax);
		}

		bool isIntersect(const Scope<T>& s) const {
			if (m_xMin > s.m_xMax || m_xMax < s.m_xMin ||
				m_yMin > s.m_yMax || m_yMax < s.m_yMin)
				return false;
			return true;
		}

		double xLength() const { return m_xMax - m_xMin; }
		double yLength() const { return m_yMax - m_yMin; }

		MapPos2D leftLow() const { return MapPos2D(m_xMin, m_yMin); }
		MapPos2D rightTop() const { return MapPos2D(m_xMax, m_yMax); }

		bool isNull() const { 
			return 
				m_xMin == std::numeric_limits<T>::max() && 
				m_xMax == std::numeric_limits<T>::min() && 
				m_yMin == std::numeric_limits<T>::max() &&
				m_yMax == std::numeric_limits<T>::min();
		}
		void setNull() {
			m_xMin = m_yMin = std::numeric_limits<T>::max();
			m_xMax = m_yMax = std::numeric_limits<T>::min();
		}

	public:
		T m_xMin;
		T m_xMax;
		T m_yMin;
		T m_yMax;
	}; // Scope

	typedef Scope<long> ScreenScope;
	typedef Scope<double> MapScope;
}

#endif // _IS_SDE_SCOPE_H_