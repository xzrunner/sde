#ifndef _IS_SDE_TEST_TEST_FILE_H_
#define _IS_SDE_TEST_TEST_FILE_H_
//#define UNICODE
#include <windows.h>

namespace IS_SDE
{
	namespace TEST
	{
		class FileUtility
		{
		public:
			static void PopFileInitializeMif(OPENFILENAME& ofn, HWND hwnd);
			static void PopFileInitializeIdx(OPENFILENAME& ofn, HWND hwnd);
			static void PopFileInitializeTxt(OPENFILENAME& ofn, HWND hwnd);
			static int PopFileOpenDlg(OPENFILENAME& ofn, HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);

		private:
			static void PopFileInitialize(OPENFILENAME& ofn, HWND hwnd);
		};
	}
}

#endif // _IS_SDE_TEST_TEST_FILE_H_