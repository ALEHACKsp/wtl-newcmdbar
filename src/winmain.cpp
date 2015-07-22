#include "stdafx.h"
#include "resource.h"
#include "mainframe.h"

CAppModule _Module;

#define INIT_OR_EXIT(expr)  \
    if (!(expr)) {                                                                      \
        AtlMessageBox(nullptr, IDS_INIT_FAILED, IDR_MAINFRAME, MB_OK | MB_ICONERROR);   \
        return 0;                                                                       \
    }

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR pCmdLine, int nCmdShow)
{
    INIT_OR_EXIT(::OleInitialize(nullptr) == S_OK);
    SCOPE_EXIT{ ::OleUninitialize(); };

    INIT_OR_EXIT(AtlInitCommonControls(ICC_WIN95_CLASSES | ICC_COOL_CLASSES));

    HMODULE hRichEdit;
    INIT_OR_EXIT(hRichEdit = ::LoadLibraryW(CRichEditCtrl::GetLibraryName()));
    SCOPE_EXIT{ ::FreeLibrary(hRichEdit); };

    INIT_OR_EXIT(_Module.Init(nullptr, hInstance) == S_OK);
    SCOPE_EXIT{ _Module.Term(); };
    
    CMessageLoop msgLoop;
    INIT_OR_EXIT(_Module.AddMessageLoop(&msgLoop));
    SCOPE_EXIT{ _Module.RemoveMessageLoop(); };

    CMainFrame wnd;
    INIT_OR_EXIT(wnd.Create());

    wnd.ToCWindow().ShowWindow(nCmdShow);
    wnd.ToCWindow().UpdateWindow();

    return msgLoop.Run();
}
