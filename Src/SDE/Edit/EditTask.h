#ifndef _IS_SDE_EDIT_EDITTASK_H_
#define _IS_SDE_EDIT_EDITTASK_H_
#include "../Tools/SpatialDataBase.h"
#include "Utility.h"
#include <windows.h>

namespace IS_SDE
{
	namespace Render
	{
		class MapLayer;
		class Selection;
	}

	namespace EDIT
	{
		class EditController;
		class EditCommand;

		class EditTask
		{
		public:
			EditTask(EditController* editController);
			virtual ~EditTask() {}

			virtual void doLButtonDown(long keyflag, long xpos, long ypos);
			virtual void doLButtonUp(long keyflag, long xpos, long ypos);
			virtual void doRButtonDown(long keyflag, long xpos, long ypos);
			virtual void doRButtonUp(long keyflag, long xpos, long ypos);
			virtual void doMouseMove(long keyflag, long xpos, long ypos);
			virtual void doLButtonDblclk(long keyflag, long xpos, long ypos);
			virtual void doMouseWheel(int iDeltaPerLine, int& iAccumDelta, long xpos, long ypos);
			virtual void doCommandLine(const EditCommand& command);

		public:
			const EditController* editController() const { return m_editController; }
			void drawMousePositoin(long xpos, long ypos) const;

		protected:
			virtual MapEditUIType taskType() = 0;
			virtual void execute();
			virtual void onPaint();
			void highlightSelection() const;
			void setEditMapLayer();

		protected:
			void transScreenToMap(long s_x, long s_y, double* d_x, double* d_y) const;
			Render::Selection* getSelection();
			void endTask();
			void refresh();

		protected:
			// only select one layers, not all 
			bool isPointSelectObjs(long xpos, long ypos);
			
			// select all layers' objs
			void pointSelectObjs(long xpos, long ypos);
			void rectSelectObjs(long x0, long y0, long x1, long y1);

		protected:
			EditController*		m_editController;
			Render::MapLayer*	m_editLayer;

			friend class EditController;

		}; // EditTask
	}
}

#endif // _IS_SDE_EDIT_EDITTASK_H_