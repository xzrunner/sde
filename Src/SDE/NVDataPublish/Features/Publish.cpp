#include "Publish.h"
#include "Condense.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Features;

Publish::Publish(std::vector<IS_SDE::Render::MapLayer*>& layers, Tools::PropertySet& ps)
{
	size_t layerSize = layers.size();
	m_layers.reserve(layerSize);
	for (size_t i = 0; i < layerSize; ++i)
	{
		CondenseData* cd = new CondenseData(layers.at(i), i, ps);
		cd->bulkLoadData(i);
		m_layers.push_back(cd);
	}
}

Publish::~Publish()
{
	for (size_t i = 0; i < m_layers.size(); ++i)
		delete m_layers.at(i);
}

void Publish::getReconstructedLayers(std::vector<NVDataPublish::STATIC_LAYER*>& layers) const
{
	layers.reserve(m_layers.size());
	for (size_t i = 0; i < m_layers.size(); ++i)
		layers.push_back(m_layers.at(i)->getReconstructedLayer());
}

void Publish::getLayersName(std::vector<std::wstring>& names) const
{
	names.clear();
	names.reserve(m_layers.size());
	for (size_t i = 0; i < m_layers.size(); ++i)
	{
		std::wstring nvrName = m_layers[i]->getLayerName();
		names.push_back(nvrName.substr(0, nvrName.find_last_of('_')) + L"_STATIC");
	}
}