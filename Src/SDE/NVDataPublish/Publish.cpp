#include "Publish.h"
#include "Arrangement/Arrange.h"
#include "Features/Publish.h"
#include "Network/Publish.h"
#include "MultiLayersNW/Publish.h"
#include "LookUpTable/Publish.h"
#include "../Render/MapLayer.h"

using namespace IS_SDE::NVDataPublish;

void Publish::publishNVData(
	HWND hwnd, 
	std::vector<IS_SDE::Render::MapLayer*>& layers, 
	Tools::PropertySet& ps
	)
{
	std::vector<ISpatialIndex*> indexes;
	indexes.reserve(layers.size());
	for (size_t i = 0; i < layers.size(); ++i)
		indexes.push_back(layers.at(i)->getSpatialIndex());

	// <1>
	Arrangement::Arrange::arrangeSrcData(hwnd, indexes, ps);
	// <2>
	Features::Publish fp(layers, ps);
	// <3>
	createSingleLayerTopo(fp, ps);
	//// <4>
	//createMultiLayersTopo(ps);
	//// <5>
	//createLookUpTable(hwnd, ps);
}

void Publish::createSingleLayerTopo(const Features::Publish& fp, Tools::PropertySet& ps)
{
	Network::Publish np(fp, ps);
	np.createNetworkData();
}

void Publish::createMultiLayersTopo(HWND hwnd, Tools::PropertySet& ps)
{
	MLNW::Publish mp(ps);
	mp.createMultiLayersNWData(hwnd);
}

void Publish::createLookUpTable(HWND hwnd, Tools::PropertySet& ps)
{
	LookUpTable::Publish lp(ps);
	lp.createRegularTable(hwnd);
}