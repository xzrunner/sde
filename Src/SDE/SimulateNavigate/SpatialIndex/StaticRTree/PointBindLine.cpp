#include "PointBindLine.h"
#include "StaticRTree.h"
#include "../../../BasicType/Point.h"
#include "../../../BasicType/Line.h"
#include "../../../NVDataPublish/Features/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::SimulateNavigate::SpatialIndex::StaticRTree;

PointBindLine::PointBindLine(
	const std::vector<ISpatialIndex*>& layers, 
	const NVDataPublish::ICondenseStrategy* cs
	) : m_condenseStrategy(cs)
{
	size_t size = layers.size();
	m_layers.reserve(size);
	for (size_t i = 0; i < size; ++i)
	{
		StaticRTree* tree = dynamic_cast<StaticRTree*>(layers.at(i));
		assert(tree);
		if (tree == NULL)
			throw Tools::IllegalStateException("PointBindLine: Layer type is not StaticRTree");
		m_layers.push_back(tree);
	}
}

void PointBindLine::bind(const MapPos2D& p, Line** line, uint64_t* id) const
{
	Point query(p);
	NNComparator nnc;

	bool find = false;
	double nearest = Tools::DOUBLE_MAX;

	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		std::priority_queue<NNEntry*, std::vector<NNEntry*>, NNEntry::ascending> queue;
		queue.push(new NNEntry(m_layers.at(i)->m_rootID, 0, 0.0));

		while (!queue.empty())
		{
			NNEntry* pFirst = queue.top();

			if (find && pFirst->m_minDist > nearest)
				break;

			queue.pop();

			if (pFirst->m_pEntry == 0)
			{
				NodePtr n = m_layers.at(i)->readNode(pFirst->m_id);

				for (size_t cChild = 0; cChild < n->m_children; cChild++)
				{
					if (n->m_bLeaf)
					{
						NVDataPublish::Features::Data* e 
							= new NVDataPublish::Features::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild], m_condenseStrategy);
						queue.push(new NNEntry(n->m_pIdentifier[cChild], e, nnc.getMinimumDistance(query, *e)));
					}
					else
					{
						queue.push(new NNEntry(n->m_pIdentifier[cChild], 0, nnc.getMinimumDistance(query, *(n->m_ptrMBR[cChild]))));
					}
				}
			}
			else
			{
				if (find && pFirst->m_minDist > nearest)
				{
					delete pFirst->m_pEntry;
					break;
				}
				else
				{
					if (find)
						delete *line;
					else
						find = true;

					IShape* pS;
					static_cast<IData*>(pFirst->m_pEntry)->getShape(&pS);
					*line = dynamic_cast<Line*>(pS);
					assert(*line);

					*id = pFirst->m_pEntry->getIdentifier();

					nearest = pFirst->m_minDist;
					delete pFirst->m_pEntry;
				}
			}

			delete pFirst;
		}

		while (!queue.empty())
		{
			NNEntry* e = queue.top(); queue.pop();
			if (e->m_pEntry != 0) delete e->m_pEntry;
			delete e;
		}
	}
}
