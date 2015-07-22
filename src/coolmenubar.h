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

// Support limited environments for now.

#if (_MSC_VER < 1500)
    #error coolmenubar.h requires _MSC_VER >= 1500
#endif

#if (_WIN32_WINNT < 0x0600)
    #error coolmenubar.h requires _WIN32_WINNT >= 0x0600
#endif

#if (_WIN32_IE < 0x0400)
    #error coolmenubar.h requires _WIN32_IE >= 0x0400
#endif

namespace WTL
{

class CCoolMenuBarBase : public WTL::CToolBarCtrl
{
};

template <class T, class TBase = CCoolMenuBarBase, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CCoolMenuBarImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
public:
    DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

    BEGIN_MSG_MAP(CCoolMenuBarImpl)
    END_MSG_MAP()

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
        return ATL::CWindowImpl< T, TBase, TWinTraits >::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
    }
};

class CCoolMenuBar : public CCoolMenuBarImpl<CCoolMenuBar>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_CoolMenuBar"), GetWndClassName())
};

} // namespace WTL

#endif // __COOLMENUBAR_H__
