#include "stdafx.h"
#include "resource.h"
#include "mainframe.h"
#include "coolmenubar.h"

class CMainFrame::Impl : 
    public CFrameWindowImpl<CMainFrame::Impl>,
    public CMessageFilter,
    public CIdleHandler
{
private:
    CCoolMenuBar    m_coolMenuBarCtrl;
    CToolBarCtrl    m_toolbarCtrl;
    
public:
    DECLARE_FRAME_WND_CLASS(L"FocuslessCommandBarApp:MainFrame", IDR_MAINFRAME);

    BEGIN_MSG_MAP(CMainFrame::Impl)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnAppExit)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame::Impl>)
    END_MSG_MAP()
    
    virtual BOOL PreTranslateMessage(MSG* pMsg) 
    {
        return CFrameWindowImpl<CMainFrame::Impl>::PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle() 
    {
        return FALSE;
    }

private:
    ////////////////////////////////////////////////////////////////////////////
    // Message Handlers

    int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        m_coolMenuBarCtrl.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
        m_coolMenuBarCtrl.AttachMenu(GetMenu());
        m_coolMenuBarCtrl.LoadImages(IDR_MAINFRAME);
        SetMenu(nullptr);

        m_toolbarCtrl = CreateSimpleToolBarCtrl(
            m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

        CreateSimpleReBar();
        AddSimpleReBarBand(m_coolMenuBarCtrl);
        AddSimpleReBarBand(m_toolbarCtrl, nullptr, TRUE);

        auto loop = _Module.GetMessageLoop();
        loop->AddMessageFilter(this);
        loop->AddIdleHandler(this);

        return 0;
    }

    void OnDestroy()
    {
        auto loop = _Module.GetMessageLoop();
        loop->RemoveMessageFilter(this);
        loop->RemoveIdleHandler(this);

        SetMsgHandled(FALSE);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Command Handlers

    void OnAppExit(UINT uNotifyCode, int nID, CWindow wndCtl)
    {
        SendMessageW(WM_CLOSE);
    }
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
    return ATL::CWindow(m_pimpl->m_hWnd);
}
