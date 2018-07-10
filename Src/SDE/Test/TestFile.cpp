#include "TestFile.h"
#include <commdlg.h>

using namespace IS_SDE::TEST;

void FileUtility::PopFileInitializeMif(OPENFILENAME& ofn, HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("MIF Files (*.MIF)\0*.mif\0")  \
							  TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = TEXT("mif");
	PopFileInitialize(ofn, hwnd);
}

void FileUtility::PopFileInitializeIdx(OPENFILENAME& ofn, HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Index Files (*.IDX)\0*.idx\0")  \
							  TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = TEXT("idx");
	PopFileInitialize(ofn, hwnd);
}

void FileUtility::PopFileInitializeTxt(OPENFILENAME& ofn, HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Txt Files (*.TXT)\0*.txt\0")  \
		TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = TEXT("txt");
	PopFileInitialize(ofn, hwnd);
}

int FileUtility::PopFileOpenDlg(OPENFILENAME& ofn, HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;

	return GetOpenFileName(&ofn) ;
}

void FileUtility::PopFileInitialize(OPENFILENAME& ofn, HWND hwnd)
{
	ofn.lStructSize       = sizeof(OPENFILENAME) ;
	ofn.hwndOwner         = hwnd ;
	ofn.hInstance         = NULL ;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter    = 0 ;
	ofn.nFilterIndex      = 0 ;
	ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL ;
	ofn.lpstrTitle        = NULL ;
	ofn.Flags             = 0 ;             // Set in Open and Close functions
	ofn.nFileOffset       = 0 ;
	ofn.nFileExtension    = 0 ;
	ofn.lCustData         = 0L ;
	ofn.lpfnHook          = NULL ;
	ofn.lpTemplateName    = NULL ;
}
