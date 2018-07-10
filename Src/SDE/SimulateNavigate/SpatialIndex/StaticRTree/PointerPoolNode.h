#ifndef _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_POINTER_POOL_NODE_H_
#define _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_POINTER_POOL_NODE_H_
#include "Node.h"
using namespace IS_SDE::SimulateNavigate::SpatialIndex;

namespace Tools
{
	template<> class PointerPool<StaticRTree::Node>
	{
	public:
		explicit PointerPool(size_t capacity) : m_capacity(capacity)
		{
#ifndef NDEBUG
			m_hits = 0;
			m_misses = 0;
			m_pointerCount = 0;
#endif
		}

		~PointerPool()
		{
			assert(m_pool.size() <= m_capacity);

			while (! m_pool.empty())
			{
				StaticRTree::Node* x = m_pool.top(); m_pool.pop();
#ifndef NDEBUG
				m_pointerCount--;
#endif
				delete x;
			}

#ifndef NDEBUG
			std::cerr << "Lost pointers: " << m_pointerCount << std::endl;
#endif
		}

		PoolPointer<StaticRTree::Node> acquire()
		{
			if (! m_pool.empty())
			{
				StaticRTree::Node* p = m_pool.top(); m_pool.pop();
#ifndef NDEBUG
				m_hits++;
#endif

				return PoolPointer<StaticRTree::Node>(p, this);
			}
#ifndef NDEBUG
			else
			{
				// fixme: well sort of...
				m_pointerCount++;
				m_misses++;
			}
#endif

			return PoolPointer<StaticRTree::Node>();
		}

		void release(StaticRTree::Node* p)
		{
			if (p != 0)
			{
				if (m_pool.size() < m_capacity)
				{
					if (p->m_pData != 0)
					{
						for (size_t cChild = 0; cChild < p->m_children; cChild++)
						{
							// there is no need to set the pointer to zero, after deleting it,
							// since it will be redeleted only if it is actually initialized again,
							// a fact that will be depicted by variable m_children.
							if (p->m_pData[cChild] != 0) delete[] p->m_pData[cChild];
						}
					}

					p->m_identifier = -1;
					p->m_children = 0;

					m_pool.push(p);
				}
				else
				{
#ifndef NDEBUG
					m_pointerCount--;
#endif
					delete p;
				}

				assert(m_pool.size() <= m_capacity);
			}
		}

		size_t getCapacity() const { return m_capacity; }
		void setCapacity(size_t c)
		{
			assert (c >= 0);
			m_capacity = c;
		}

	protected:
		size_t m_capacity;
		std::stack<StaticRTree::Node*> m_pool;

#ifndef NDEBUG
	public:
		size_t m_hits;
		size_t m_misses;
		size_t m_pointerCount;
#endif
	};
}

#endif // _IS_SDE_SIMULATE_NAVIGATE_SPATIALINDEX_STATIC_RTREE_POINTER_POOL_NODE_H_
