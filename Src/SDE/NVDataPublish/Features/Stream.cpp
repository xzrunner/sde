#include "Stream.h"
#include "EdgeIDTransform.h"
#include "../../SpatialIndex/NVRTree/NVRTree.h"
#include "../../SpatialIndex/Utility/Data.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Features;

//
// class LayerStream
//

LayerStream::LayerStream(ISpatialIndex* layer, size_t layerID, const NVDataPublish::ICondenseStrategy* cs)
	: m_currIndex(0), m_layerID(layerID), m_condenseStrategy(cs)
{
	m_currNode.relinquish();
	m_layer = dynamic_cast<NVRTree::NVRTree*>(layer);
	if (m_layer == NULL)
	{
		assert(0);
		throw Tools::IllegalStateException(
			"LayerStream::LayerStream: layer should be NVRTree. "
			);
	}

	m_ids.push(m_layer->m_rootID);
	setNextData();
}

LayerStream::~LayerStream()
{
	if (m_pNext != NULL)
		delete m_pNext;
}

IData* LayerStream::getNext()
{
	if (m_pNext == NULL)
		return NULL;

	SpatialIndex::Data* ret = m_pNext;
	setNextData();
	return ret;
}

bool LayerStream::hasNext()
{
	return m_pNext != NULL;
}

size_t LayerStream::size()
{
	throw Tools::NotSupportedException("LayerStream::size: Operation not supported.");
}

void LayerStream::rewind()
{
	m_currNode.relinquish();
	m_currIndex = 0;

	m_ids.push(m_layer->m_rootID);
	setNextData();
}

void LayerStream::setNextData()
{
	if (m_currNode.get() && m_currIndex < m_currNode->getChildrenCount())
	{
		IShape* s;
		m_currNode->getChildShape(m_currIndex, &s);

		id_type id = EdgeIDTransform::getIDWithLayerInfo(
			m_currNode->getChildIdentifier(m_currIndex), m_layerID
			);

		m_pNext = new SpatialIndex::Data(s, id);
		delete s;

		++m_currIndex;
	}
	else
	{
		while (!m_ids.empty())
		{
			id_type nextNodeID = m_ids.front(); m_ids.pop();
			m_currNode = m_layer->readNode(nextNodeID);

			if (m_currNode->isIndex())
			{
				for (size_t cChild = 0; cChild < m_currNode->getChildrenCount(); cChild++)
				{
					m_ids.push(m_currNode->getChildIdentifier(cChild));
				}
			}
			else
			{
				m_currIndex = 0;

				IShape* s;
				m_currNode->getChildShape(m_currIndex, &s);

				id_type id = EdgeIDTransform::getIDWithLayerInfo(
					m_currNode->getChildIdentifier(m_currIndex), m_layerID
					);

				m_pNext = new SpatialIndex::Data(s, id);
				delete s;

				++m_currIndex;

				return;
			}
		}

		m_pNext = NULL;
	}
}