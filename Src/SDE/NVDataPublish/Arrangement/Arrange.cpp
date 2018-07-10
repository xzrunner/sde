#include "Arrange.h"
#include "MapPartition.h"
#include "Point.h"
#include "Line.h"
#include "RoadsBreakIn.h"
#include "../../SpatialIndex/Utility/QueryStrategy.h"

using namespace IS_SDE;
using namespace IS_SDE::NVDataPublish::Arrangement;

void Arrange::arrangeSrcData(
	HWND hwnd, 
	const std::vector<ISpatialIndex*>& layers, 
	Tools::PropertySet& ps
	)
{
	Tools::Variant var;

	// max number of roads' each time to compute
	var = ps.getProperty("MaxComputeRoadNum");
	if (var.m_varType != Tools::VT_ULONG || var.m_val.ulVal < 2)
		throw Tools::IllegalArgumentException(
		"NVDataPublish::publishData: Property MaxComputeNum must be Tools::VT_ULONG and >= 2"
		);
	size_t maxComputeNum = var.m_val.lVal;

	// tolerance for compute connection
	var = ps.getProperty("Precision");
	if (var.m_varType != Tools::VT_DOUBLE)
		throw Tools::IllegalArgumentException(
		"NVDataPublish::publishData: Property Precision must be Tools::VT_DOUBLE"
		);
	double precision = std::max(var.m_val.dblVal, 0.0);

	// test
	clock_t totStart = clock(); 
	//

	PartitionMap pm(layers, maxComputeNum);

	// test
	size_t iLayer;
	//

	// test
	double scanningTime = 0, eraseTime = 0, breakAndConnectTime = 0, updateTime = 0,
		totTime = 0, queryTime = 0, initTime = 0, otherTime = 0;
	//

	NodeMgr nodeMgr;
	LineMgr lineMgr;

	size_t computeGridCount = pm.m_allLeaves.size();
	for (size_t i = 0; i < computeGridCount; ++i)
	{
		// display progress
		char positionBuffer[100];
		sprintf(positionBuffer, "%d / %d", i, computeGridCount);
		HDC hdc = GetDC(hwnd);
		TextOutA(hdc, 10, 0, positionBuffer, strlen(positionBuffer));
		ReleaseDC(hwnd, hdc);
		//

		// test
		iLayer = i;
		//

		Rect computeRegion = pm.m_allLeaves.at(i)->m_scope;
		computeRegion.changeSize(precision, precision);

		//// debug
		//MapPos2D test(10.755503, 35.090248);
		
		//assert(computeRegion.containsPoint(test));
		////if (!computeRegion.containsPoint(test))
		////	continue;
		////

		//// debug
		//double low[2], high[2];
		//low[0] = 10.653306;
		//high[0] = 10.853306;
		//low[1] = 34.991927;
		//high[1] = 35.191927;
		//Rect test(low, high);
		//if (!computeRegion.intersectsRect(test))
		//	continue;
		////

		// test
		clock_t start = clock(); 
		//

		Rect dataRegion = computeRegion;
		SpatialIndex::GetSelectedGridsMBRVisitor vis(&dataRegion);
		for (size_t j = 0; j < layers.size(); ++j)
			layers.at(j)->intersectsWithQuery(computeRegion, vis, false);
		// test
		queryTime += clock() - start;
		start = clock();
		//

		nodeMgr.init(dataRegion, precision);
		lineMgr.init(precision, computeRegion, layers, &nodeMgr);
		// test
		initTime += clock() - start;
		start = clock();
		//

		double pregress;
		RoadsBreakIn breakIn(&nodeMgr, precision);
		breakIn.handleEventList(&pregress);
		// test
		scanningTime += clock() - start;
		start = clock();
		//

		lineMgr.eraseDuplicateRoads();
		// test
		eraseTime += clock() - start;
		start = clock();
		//

		// For break in & connect
		nodeMgr.setAllNodesType(computeRegion);
		lineMgr.breakInRoads();
		lineMgr.connectRoads();

		// test
		breakAndConnectTime += clock() - start;
		start = clock();
		//

		lineMgr.updateToDatabase(layers);
		// test
		updateTime += clock() - start;
		start = clock();
		//

		nodeMgr.clear();
		lineMgr.clear();
	}

	// test
	totTime = clock() - totStart;
	otherTime = totTime - queryTime - initTime - scanningTime - eraseTime
		- breakAndConnectTime - updateTime;

	std::ofstream fout("c:/PD_7_10.txt", std::ios_base::app);
	fout << std::setw(20) << "Query:" << queryTime << std::setw(12) << queryTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Init:" << initTime << std::setw(12) << initTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Scanning:" << scanningTime << std::setw(12) << scanningTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Erase:" << eraseTime << std::setw(12) << eraseTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Break & connect:" << breakAndConnectTime << std::setw(12) << breakAndConnectTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Update:" << updateTime << std::setw(12) << updateTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Other:" << otherTime << std::setw(12) << otherTime * 100 / totTime << "%" << std::endl;
	fout << std::setw(20) << "Tot:" << totTime << std::endl;
	//
}