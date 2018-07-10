#ifndef _IS_SDE_BASIC_GM_TYPE_H_
#define _IS_SDE_BASIC_GM_TYPE_H_
namespace IS_SDE
{
	//////////////////////////////////////////////////////////////////////////
	// Pos2D
	//////////////////////////////////////////////////////////////////////////

	template<class T>
	class Pos2D
	{
	public:
		T d0, d1;

	public:
		Pos2D() {}
		Pos2D(T x, T y) : d0(x), d1(y) {}
		Pos2D(const Pos2D &right) : d0(right.d0), d1(right.d1) {}

		bool operator == (const Pos2D &right) const {	
			return d0 == right.d0 && d1 == right.d1;	
		}

		bool operator < (const Pos2D &right) const {	
			return d0 < right.d0 
				|| d0 == right.d0 && d1 > right.d1;	
		}

		bool operator != (const Pos2D &right) const {	
			return !(*this == right);	
		}

		Pos2D& operator = (const Pos2D &right) {
			d0 = right.d0; d1 = right.d1 ;
			return *this;
		}
	};	// Pos2D

	template<class T>
	class PosCmp
	{
	public:
		bool operator () (const Pos2D<T>& lhs, const Pos2D<T>&  rhs) const {
			return lhs.d0 < rhs.d0
				|| lhs.d0 == rhs.d0 && lhs.d1 < rhs.d1;
		}
	}; // PosCmp

	typedef Pos2D<long> ScreenPos2D;
	typedef Pos2D<double> MapPos2D;

	//////////////////////////////////////////////////////////////////////////
	// Pos3D
	//////////////////////////////////////////////////////////////////////////

	template<class T>
	class Pos3D
	{
	public:
		T d0, d1, d2;

	public:
		Pos3D() {}
		Pos3D(T x, T y, T z) : d0(x), d1(y), d2(z) {}
		Pos3D(const Pos3D& right) : d0(right.d0), d1(right.d1), d2(right.d2) {}

		bool operator == (const Pos3D &right) const {	
			return d0 == right.d0 && d1 == right.d1 && d2 == right.d2;
		}

		bool operator != (const Pos3D &right) const {	
			return !(*this == right);	
		}

		Pos3D& operator = (const Pos3D &right) {
			d0 = right.d0; d1 = right.d1; d2 = right.d2;
			return *this;
		}
	};	// Pos3D

	typedef Pos3D<long> ScreenPos3D;
	typedef Pos3D<double> MapPos3D;

}
#endif	// _IS_SDE_BASIC_GM_TYPE_H_