#ifndef _IS_SDE_EDIT_PUBLISH_NV_DATA_TASK_H_
#define _IS_SDE_EDIT_PUBLISH_NV_DATA_TASK_H_
#include "../EditTask.h"

namespace IS_SDE
{
	namespace EDIT
	{
		class PublishNVDataTask : public EditTask
		{
		public:
			PublishNVDataTask(EditController* editController);

			virtual void doMouseMove(long keyflag, long xpos, long ypos);
			virtual void doLButtonDblclk(long keyflag, long xpos, long ypos);

		protected:
			virtual MapEditUIType taskType() { return PUBLISH_NV_DATA_TASK; }
			virtual void onPaint();

		private:
			virtual void execute();

		private:
			const double m_precision;

		}; // PublishNVDataTask
	}
}

#endif // _IS_SDE_EDIT_PUBLISH_NV_DATA_TASK_H_