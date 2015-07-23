#ifndef __COOLMENUBAR_H__
#define __COOLMENUBAR_H__

#pragma once

#ifdef _WIN32_WCE
    #error coolmenubar.h is not supported on Windows CE
#endif

#ifndef __ATLAPP_H__
    #error coolmenubar.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
    #error coolmenubar.h requires atlctrls.h to be included first
#endif

#ifndef __ATLTHEME_H__
    #error coolmenubar.h requires atltheme.h to be included first
#endif

// Support limited environments for now.

#if (_MSC_VER < 1500)
    #error coolmenubar.h requires _MSC_VER >= 1500
#endif

#if (_WIN32_WINNT < 0x0600)
    #error coolmenubar.h requires _WIN32_WINNT >= 0x0600
#endif

#if (_WIN32_IE < 0x0500)
    #error coolmenubar.h requires _WIN32_IE >= 0x0500
#endif

namespace WTL
{

class CCoolMenuBarBase : public CToolBarCtrl
{
};

template <class T, class TBase = CCoolMenuBarBase, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CCoolMenuBarImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
public:
    DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

    // Constants

    enum _CmdBarDrawConstants
    {
        s_kcxGap = 1,
        s_kcxTextMargin = 2,
        s_kcxButtonMargin = 3,
        s_kcyButtonMargin = 3
    };

    enum
    {
        _chChevronShortcut = _T('/')
    };

    // Data members

    HMENU m_hMenu;
    ATL::CContainedWindow m_wndParent;

    bool m_bParentActive : 1;
    bool m_bShowKeyboardCues : 1;
    // Constructor/destructor

    CCoolMenuBarImpl() :
        m_hMenu(NULL),
        m_wndParent(this, 1),
        m_bParentActive(true),
        m_bShowKeyboardCues(false)
    {
    }

    ~CCoolMenuBarImpl()
    {
        if (m_hMenu != NULL/* && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0*/)
            ::DestroyMenu(m_hMenu);
    }

    // Attributes

    CMenuHandle GetMenu() const
    {
        ATLASSERT(::IsWindow(m_hWnd));
        return m_hMenu;
    }

    // Methods

    HWND Create(HWND hWndParent,
                RECT& rcPos,
                LPCTSTR szWindowName = NULL,
                DWORD dwStyle = 0,
                DWORD dwExStyle = 0,
                UINT nID = 0,
                LPVOID lpCreateParam = NULL)
    {
        // These styles are required for command bars
        dwStyle |= TBSTYLE_LIST | TBSTYLE_FLAT;

        return ATL::CWindowImpl< T, TBase, TWinTraits >::Create(
            hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
    }

    BOOL AttachMenu(HMENU hMenu)
    {
        ATLASSERT(::IsWindow(m_hWnd));
        ATLASSERT(hMenu == NULL || ::IsMenu(hMenu));
        if (hMenu != NULL && !::IsMenu(hMenu))
            return FALSE;

        // destroy old menu, if needed, and set new one
        if (m_hMenu != NULL/* && (m_dwExtendedStyle & CBR_EX_SHAREMENU) == 0*/)
            ::DestroyMenu(m_hMenu);

        m_hMenu = hMenu;

        // Build buttons according to menu

        SetRedraw(FALSE);

        // Clear all buttons
        int nCount = GetButtonCount();
        for (int i = 0; i < nCount; ++i)
            ATLVERIFY(DeleteButton(0) != FALSE);

        // Add buttons for each menu item
        if (m_hMenu != NULL)
        {
            int nItems = ::GetMenuItemCount(m_hMenu);

            T* pT = static_cast<T*>(this);
            pT;   // avoid level 4 warning

            for (int i = 0; i < nItems; ++i)
            {
                // Get the text length first.

                CMenuItemInfo mii;
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_STRING;

                BOOL bRet = ::GetMenuItemInfo(m_hMenu, i, TRUE, &mii);
                ATLASSERT(bRet);

                // Then, get the menu item info using a sufficient buffer.

                ATL::CTempBuffer<TCHAR> szString;
                szString.Allocate(mii.cch + 1);

                mii.fMask      = MIIM_TYPE | MIIM_STATE | MIIM_SUBMENU;
                mii.fType      = MFT_STRING;
                mii.dwTypeData = szString;
                mii.cch        = mii.cch + 1;

                bRet = ::GetMenuItemInfo(m_hMenu, i, TRUE, &mii);
                ATLASSERT(bRet);

                // NOTE: Currently supports only drop-down menu items.
                ATLASSERT(mii.hSubMenu != NULL);

                // Add a button corresponds the i-th menu item.

                TBBUTTON btn = { 0 };
                btn.iBitmap   = 0;
                btn.idCommand = i;
                btn.fsState   = (BYTE)(((mii.fState & MFS_DISABLED) == 0) ? TBSTATE_ENABLED : 0);
                btn.fsStyle   = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_DROPDOWN;
                btn.dwData    = 0;
                btn.iString   = 0;

                bRet = InsertButton(-1, &btn);
                ATLASSERT(bRet);

                TBBUTTONINFO bi = { 0 };
                bi.cbSize  = sizeof(TBBUTTONINFO);
                bi.dwMask  = TBIF_TEXT;
                bi.pszText = szString;

                bRet = SetButtonInfo(i, &bi);
                ATLASSERT(bRet);
            }
        }

        SetRedraw(TRUE);
        Invalidate();
        UpdateWindow();

        return TRUE;
    }

    // Message map and handlers

    BEGIN_MSG_MAP(CCoolMenuBarImpl)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    ALT_MSG_MAP(1)   // Parent window messages
        MESSAGE_HANDLER(WM_ACTIVATE, OnParentActivate)
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
    END_MSG_MAP()

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        // Let the toolbar initialize itself.

        LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

        // Intercept the parent window's messages.

        ATL::CWindow wndParent = GetParent();
        ATL::CWindow wndTopLevelParent = wndParent.GetTopLevelParent();
        m_wndParent.SubclassWindow(wndTopLevelParent);

        // Initialize the toolbar.

        SetButtonStructSize();
        SetImageList(NULL);

        return lRet;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

        // Stop intercepting the parent window's messages.

        if(m_wndParent.IsWindow())
            m_wndParent.UnsubclassWindow();

        return lRet;
    }

    LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        // Always transparent for now.

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnParentActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        m_bParentActive = (LOWORD(wParam) != WA_INACTIVE);

        bHandled = FALSE;
        return 1;
    }


    LRESULT OnParentCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        LRESULT lRet = CDRF_DODEFAULT;
        bHandled = FALSE;

        if (pnmh->hwndFrom != m_hWnd)
            return lRet;

        LPNMTBCUSTOMDRAW lpTBCustomDraw = (LPNMTBCUSTOMDRAW)pnmh;
        if (lpTBCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
        {
            lRet = CDRF_NOTIFYITEMDRAW;
            bHandled = TRUE;
        }
        else if (lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
        {
            // Prevent only the text from being drawn.

            ::SetRectEmpty(&lpTBCustomDraw->rcText);

            lRet = CDRF_NOTIFYPOSTPAINT;
            bHandled = TRUE;
        }
        else if (lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
        {
            if (!m_bParentActive || (lpTBCustomDraw->nmcd.uItemState & CDIS_DISABLED))
                lpTBCustomDraw->clrText = ::GetSysColor(COLOR_GRAYTEXT);

            _ParentCustomDrawHelper(lpTBCustomDraw);

            lRet = CDRF_SKIPDEFAULT;
            bHandled = TRUE;
        }

        return lRet;
    }

    void _ParentCustomDrawHelper(LPNMTBCUSTOMDRAW lpTBCustomDraw)
    {
        // Draw the text at the same position even if the button is pressed.

        CDCHandle dc = lpTBCustomDraw->nmcd.hdc;
        dc.SetTextColor(lpTBCustomDraw->clrText);
        dc.SetBkMode(lpTBCustomDraw->nStringBkMode);

        HFONT hFont = GetFont();
        HFONT hFontOld = NULL;
        if (hFont != NULL)
            hFontOld = dc.SelectFont(hFont);

        const int cchText = 200;
        TCHAR szText[cchText] = { 0 };
        TBBUTTONINFO tbbi = { 0 };
        tbbi.cbSize = sizeof(TBBUTTONINFO);
        tbbi.dwMask = TBIF_TEXT;
        tbbi.pszText = szText;
        tbbi.cchText = cchText;
        GetButtonInfo((int)lpTBCustomDraw->nmcd.dwItemSpec, &tbbi);

        dc.DrawText(szText, -1, &lpTBCustomDraw->nmcd.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | (m_bShowKeyboardCues ? 0 : DT_HIDEPREFIX));

        if (hFont != NULL)
            dc.SelectFont(hFontOld);
    }
};

class CCoolMenuBar : public CCoolMenuBarImpl<CCoolMenuBar>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_CoolMenuBar"), GetWndClassName())
};

} // namespace WTL

#endif // __COOLMENUBAR_H__
