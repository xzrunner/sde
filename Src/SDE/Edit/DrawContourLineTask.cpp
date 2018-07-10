//#include "DrawContourLineTask.h"
//#include "../SpatialIndex/Utility/QueryStrategy.h"
//#include "../BasicType/BasicGMType.h"
//#include "../BasicType/Point3D.h"
//#include "EditController.h"
//#include "../Render/MapLayer.h"
//#include "../Render/MapView.h"
//#include "../Render/Render.h"
//
//using namespace IS_SDE;
//using namespace IS_SDE::EDIT;
//
//DrawContourLineTask::DrawContourLineTask(EditController* editController)
//	: EditTask(editController)
//{
//}
//
//void DrawContourLineTask::doLButtonDown(long keyflag, long xpos, long ypos)
//{
////	DialogBox(m_editController->m_hInstance, TEXT("SELECTHEIGHTBOX"), hwnd, SelectAttrProc);
//
//	double realCoords[3];
//	m_editController->getMapView()->transScreenToMap(xpos, ypos, &realCoords[0], &realCoords[1]);
//	realCoords[2] = 0.0;
//	Point3D p(realCoords);
//
//	byte* buffer;
//	size_t dataLength;
//	p.storeToByteArray(&buffer, dataLength);
//
//	id_type objID = m_editLayer->getSpatialIndex()->nextObjID(), nodeID;
//	m_editLayer->getSpatialIndex()->insertData(dataLength, buffer, p, objID, &nodeID);
//	delete[] buffer;
//
//	m_tin.insert(realCoords);
//
//	InvalidateRect(m_editController->getHWND(), NULL, true);
//}
//
//void DrawContourLineTask::doLButtonUp(long keyflag, long xpos, long ypos)
//{
//}
//
//void DrawContourLineTask::execute()
//{
//	if (m_tin.empty())
//	{
//		SpatialIndex::GetAllPoint3DQueryStrategy qs;
//		m_editLayer->getSpatialIndex()->queryStrategy(qs);
//
//		if (qs.m_points.empty())
//		{
//			MessageBox(m_editController->getHWND(), TEXT("No Point3D in edit layer. "), TEXT(""), 0);
//			return;
//		}
//
//		size_t posSize = qs.m_points.size();
//		std::vector<double> height;
//		std::vector<MapPos2D> allPos;
//		height.reserve(posSize);
//		allPos.reserve(posSize);
//
//		for (size_t i = 0; i < posSize; ++i)
//		{
//			height.push_back(*qs.m_points.at(i)->m_pHeight);
//			allPos.push_back(MapPos2D(qs.m_points.at(i)->m_pCoords[0], qs.m_points.at(i)->m_pCoords[1]));
//		}
//
//		m_tin.createDelaunayTriStructure(allPos, height);
//	}
//
//	std::vector<std::vector<MapPos2D> > triangles;
//	m_tin.getAllTriangles(&triangles);
//
//	std::vector<std::pair<double, double> > ranges;
//
//	//double lowRange = m_tin.m_dt.m_low - 1.5;
//	//while (lowRange < m_tin.m_dt.m_high + 1.5)
//	//{
//	//	ranges.push_back(std::make_pair(lowRange, lowRange + 0.3));
//	//	lowRange += 0.3;
//	//}
//
//	ranges.push_back(std::make_pair(-100, -10));
//	ranges.push_back(std::make_pair(-10, -5));
//	ranges.push_back(std::make_pair(-5, -1));
//	ranges.push_back(std::make_pair(-1, -0.5));
//	ranges.push_back(std::make_pair(-0.5, 0));
//	ranges.push_back(std::make_pair(0, 0.3));
//	ranges.push_back(std::make_pair(0.3, 0.5));
//	ranges.push_back(std::make_pair(1, 1.5));
//	ranges.push_back(std::make_pair(1.5, 2));
//	ranges.push_back(std::make_pair(2, 2.5));
//	ranges.push_back(std::make_pair(2.5, 3));
//	ranges.push_back(std::make_pair(3, 4));
//	ranges.push_back(std::make_pair(4, 5));
//	ranges.push_back(std::make_pair(5, 6));
//	ranges.push_back(std::make_pair(6, 20));
//
//
//	std::vector<std::vector<std::vector<MapPos2D> > > layersFaces;
//	m_tin.getContourFaces(ranges, &layersFaces);
//
//	m_editController->getRender()->startMemRender();
//
//	for (size_t i = 0; i < triangles.size(); ++i)
//		m_editController->getRender()->renderLine(triangles.at(i));
//	for (size_t i = 0; i < layersFaces.size(); ++i)
//	{
//		for (size_t j = 0; j < layersFaces.at(i).size(); ++j)
//			m_editController->getRender()->renderFace(layersFaces.at(i).at(j));
//		m_editController->getRender()->endMemRender();
//		Sleep(300);
//	}
//}
//
//void DrawContourLineTask::onPaint()
//{
////	execute();
//}