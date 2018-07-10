//#ifndef _IS_SDE_EDIT_DRAW_CONTOUR_LINE_TASK_H_
//#define _IS_SDE_EDIT_DRAW_CONTOUR_LINE_TASK_H_
//#include "EditTask.h"
//
//namespace IS_SDE
//{
//	namespace EDIT
//	{
//		class DrawContourLineTask : public EditTask
//		{
//		public:
//			DrawContourLineTask(EditController* editController);
//
//			virtual void doLButtonDown(long keyflag, long xpos, long ypos);
//			virtual void doLButtonUp(long keyflag, long xpos, long ypos);
//
//		protected:
//			virtual MapEditUIType taskType() { return DRAW_CONTOUR_LINE_TASK; }
//			virtual void execute();
//			virtual void onPaint();
//
//		private:
//			Algorithms::SurfaceFeatureUsingTIN m_tin;
//
//	//		Algorithms::ALG_DelaunayTriangulation<MapPos2D> m_dt;
//
////			long m_firstX, m_firstY;
//
//		}; // DrawContourLineTask
//	}
//}
//
//#endif // _IS_SDE_EDIT_DRAW_CONTOUR_LINE_TASK_H_