#ifndef _IS_SDE_NVDATAPUBLISH_FEATURES_PUBLISH_H_
#define _IS_SDE_NVDATAPUBLISH_FEATURES_PUBLISH_H_
#include "../Base.h"

namespace IS_SDE
{
	namespace Render
	{
		class MapLayer;
	}

	namespace NVDataPublish
	{
		namespace Features
		{
			class CondenseData;

			class Publish
			{
			public:
				Publish(std::vector<Render::MapLayer*>& layers, Tools::PropertySet& ps);
				~Publish();

				void getReconstructedLayers(std::vector<STATIC_LAYER*>& layers) const;
				void getLayersName(std::vector<std::wstring>& names) const;

			private:
				std::vector<CondenseData*> m_layers;

			}; // Publish
		}
	}
}

#endif // _IS_SDE_NVDATAPUBLISH_FEATURES_PUBLISH_H_