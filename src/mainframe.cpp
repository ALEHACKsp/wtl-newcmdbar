#include "stdafx.h"
#include "resource.h"
#include "mainframe.h"

class CMainFrame::Impl : public CFrameWindowImpl<CMainFrame::Impl>
{
public:
    DECLARE_FRAME_WND_CLASS(L"FocuslessCommandBarApp:MainFrame", IDR_MAINFRAME);

    BEGIN_MSG_MAP(CMainFrame::Impl)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame::Impl>)
    END_MSG_MAP()
};

CMainFrame::CMainFrame() : m_pimpl(std::make_unique<Impl>())
{
}

CMainFrame::~CMainFrame()
{
}

HWND CMainFrame::Create()
{
    return m_pimpl->CreateEx();
}

ATL::CWindow CMainFrame::ToCWindow() const
{
    return CWindow(m_pimpl->m_hWnd);
}
