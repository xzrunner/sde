#ifndef _IS_SDE_EDIT_DISPLAY_VIEW_TASK_H_
#define _IS_SDE_EDIT_DISPLAY_VIEW_TASK_H_
#include "EditTask.h"

namespace IS_SDE
{
	namespace EDIT
	{
		class DisplayViewTask : public EditTask
		{
		public:
			DisplayViewTask(EditController* editController);

			virtual void doLButtonDown(long keyflag, long xpos, long ypos);
			virtual void doMouseMove(long keyflag, long xpos, long ypos);

		protected:
			virtual MapEditUIType taskType() { return DISPLAY_VIEW_TASK; }
			virtual void onPaint();

		private:
			bool m_bFinishFirstPoint;
			long m_firstX, m_firstY;

		}; // DisplayViewTask
	}
}

#endif // _IS_SDE_EDIT_DISPLAY_VIEW_TASK_H_