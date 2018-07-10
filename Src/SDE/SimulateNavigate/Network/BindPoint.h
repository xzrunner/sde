#ifndef _IS_SDE_SIMULATE_NAVIGATE_NETWORK_BIND_POINT_H_
#define _IS_SDE_SIMULATE_NAVIGATE_NETWORK_BIND_POINT_H_
#include "../NavigateBase.h"

namespace IS_SDE
{
	namespace SimulateNavigate
	{
		namespace Network
		{
			class SingleLayerNW;

			class BindPoint
			{
			public:
				MapPos2D m_pos;
				MapPos2D m_beginPos, m_endPos;
				size_t m_beginID, m_endID;
				double m_beginLen, m_endLen;
				uint64_t m_roadID;

			public:
				BindPoint(const MapPos2D& p, const IPointBindLine* bind, const SingleLayerNW* nw);
				BindPoint(std::ifstream& fin);

				void output(std::ofstream& fout) const;
			};
		}
	}
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_NETWORK_BIND_POINT_H_