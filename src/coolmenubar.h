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

    // Declarations

    struct _MenuItemData	// menu item data
    {
        DWORD dwMagic;
        LPTSTR lpstrText;
        UINT fType;
        UINT fState;
        int iButton;

        _MenuItemData() { dwMagic = 0x1313; }
        bool IsCmdBarMenuItem() { return (dwMagic == 0x1313); }
    };

    struct _ToolBarData	// toolbar resource data
    {
        WORD wVersion;
        WORD wWidth;
        WORD wHeight;
        WORD wItemCount;
        WORD aItems[1];

        WORD* items()
        {
            return &aItems[0];
        }
    };

    // Constants

    //enum _CmdBarDrawConstants
    //{
    //    s_kcxGap = 1,
    //    s_kcxTextMargin = 2,
    //    s_kcxButtonMargin = 3,
    //    s_kcyButtonMargin = 3
    //};

    enum
    {
        _chChevronShortcut = _T('/'),
        _nDefaultBitmapWidth  = 16,
        _nDefaultBitmapHeight = 15,
    };

    // Data members

    HMENU                   m_hMenu;
    HIMAGELIST              m_hImageList;
    ATL::CSimpleArray<WORD> m_arrCommand;   // Command IDs, corresponds to the image list.

    ATL::CContainedWindow m_wndParent;

    bool m_bParentActive        : 1;
    bool m_bShowKeyboardCues    : 1;
    bool m_bAlphaImages         : 1;

    SIZE m_szBitmap;
    //SIZE m_szButton;

    COLORREF m_clrMask;

    // Constructor/destructor

    CCoolMenuBarImpl() :
        m_hMenu(NULL),
        m_hImageList(NULL),
        m_wndParent(this, 1),
        m_bParentActive(true),
        m_bShowKeyboardCues(false),
        m_bAlphaImages(false),
        m_clrMask(RGB(192, 192, 192))
    {
        SetImageSize(_nDefaultBitmapWidth, _nDefaultBitmapHeight);
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

    COLORREF GetImageMaskColor() const
    {
        return m_clrMask;
    }

    COLORREF SetImageMaskColor(COLORREF clrMask)
    {
        COLORREF clrOld = m_clrMask;
        m_clrMask = clrMask;
        return clrOld;
    }

    void GetImageSize(SIZE& size) const
    {
        size = m_szBitmap;
    }

    bool SetImageSize(SIZE& size)
    {
        return SetImageSize(size.cx, size.cy);
    }

    bool SetImageSize(int cx, int cy)
    {
        if (m_hImageList != NULL)
        {
            if (::ImageList_GetImageCount(m_hImageList) == 0)   // empty
            {
                ::ImageList_Destroy(m_hImageList);
                m_hImageList = NULL;
            }
            else
            {
                return false;   // can't set, image list exists
            }
        }

        if (cx == 0 || cy == 0)
            return false;

        m_szBitmap.cx = cx;
        m_szBitmap.cy = cy;
        //m_szButton.cx = m_szBitmap.cx + 2 * s_kcxButtonMargin;
        //m_szButton.cy = m_szBitmap.cy + 2 * s_kcyButtonMargin;

        return true;
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

    BOOL LoadImages(ATL::_U_STRINGorID image)
    {
        return _LoadImagesHelper(image, false);
    }

    BOOL LoadMappedImages(UINT nIDImage, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
    {
        return _LoadImagesHelper(nIDImage, true, nFlags, lpColorMap, nMapSize);
    }

    BOOL _LoadImagesHelper(ATL::_U_STRINGorID image, bool bMapped, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        // Load and parse the toolbar resource.

        CResource src;
        if (!src.Load(RT_TOOLBAR, image))
            return FALSE;

        _ToolBarData* pData = (_ToolBarData*)src.Lock();
        if (pData == NULL)
            return FALSE;

        ATLASSERT(pData->wVersion == 1);


        // Set internal data.

        SetImageSize(pData->wWidth, pData->wHeight);

        // Create image list if needed.

        if (m_hImageList == NULL)
        {
            // Check if the bitmap is 32-bit (alpha channel) bitmap (valid for Windows XP only)
            T* pT = static_cast<T*>(this);
            m_bAlphaImages = AtlIsAlphaBitmapResource(image);

            if (!pT->CreateInternalImageList(pData->wItemCount))
                return FALSE;
        }

        CImageList imageList = m_hImageList;

        // Add bitmap to our image list.

        CBitmap bmp;
        if (bMapped)
        {
            ATLASSERT(HIWORD(PtrToUlong(image.m_lpstr)) == 0);   // if mapped, must be a numeric ID
            int nIDImage = (int)(short)LOWORD(PtrToUlong(image.m_lpstr));
            bmp.LoadMappedBitmap(nIDImage, (WORD)nFlags, lpColorMap, nMapSize);
        }
        else
        {
            if (m_bAlphaImages)
                bmp = AtlLoadBitmapImage(image, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
            else
                bmp.LoadBitmap(image);
        }

        ATLASSERT(bmp.m_hBitmap != NULL);
        if (bmp.m_hBitmap == NULL)
            return FALSE;

        if (::ImageList_AddMasked(m_hImageList, bmp, m_clrMask) == -1)
            return FALSE;

        // Fill the array with command IDs.

        for (int i = 0; i < pData->wItemCount; ++i)
        {
            if (pData->aItems[i] != 0)
                m_arrCommand.Add(pData->aItems[i]);
        }

        int nImageCount = ::ImageList_GetImageCount(m_hImageList);
        ATLASSERT(nImageCount == m_arrCommand.GetSize());
        if (nImageCount != m_arrCommand.GetSize())
            return FALSE;

        return TRUE;
    }

    // Message map and handlers

    BEGIN_MSG_MAP(CCoolMenuBarImpl)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    ALT_MSG_MAP(1)   // Parent window messages
        MESSAGE_HANDLER(WM_ACTIVATE, OnParentActivate)
        NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnParentDropDown)
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnParentCustomDraw)
    END_MSG_MAP()

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        // Let the toolbar initialize itself.

        LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

        // Subclass the parent window to receive the notifications to it without
        // the message/notification reflection.

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

    LRESULT OnParentDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        // Check if this comes from us.

        if (pnmh->hwndFrom != m_hWnd)
        {
            bHandled = FALSE;
            return TBDDRET_NODEFAULT;
        }

        LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pnmh;
        int nIndex = CommandToIndex(pNMToolBar->iItem);

        // get popup menu and it's position
        RECT rect = { 0 };
        GetItemRect(nIndex, &rect);

        POINT pt = { rect.left, rect.bottom };
        MapWindowPoints(NULL, &pt, 1);
        MapWindowPoints(NULL, &rect);

        TPMPARAMS TPMParams = { 0 };
        TPMParams.cbSize    = sizeof(TPMPARAMS);
        TPMParams.rcExclude = rect;

        HMENU hMenuPopup = ::GetSubMenu(m_hMenu, nIndex);
        ATLASSERT(hMenuPopup != NULL);

        // get button ID

        TBBUTTON tbb = { 0 };
        GetButton(nIndex, &tbb);

        const int nCmdID = tbb.idCommand;

        PressButton(nCmdID, TRUE);
        SetHotItem(nCmdID);

        ::TrackPopupMenuEx(hMenuPopup,
            TPM_LEFTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION,
            pt.x, pt.y, m_hWnd, &TPMParams);

        PressButton(nCmdID, FALSE);

        return TBDDRET_DEFAULT;
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
            // Let the toolbar to draw the button normally, except for the text.

            ::SetRectEmpty(&lpTBCustomDraw->rcText);

            lRet = CDRF_NOTIFYPOSTPAINT;
            bHandled = TRUE;
        }
        else if (lpTBCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
        {
            // Draw the text at the same position even if the button is pressed down.

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

        UINT format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
        if (!m_bShowKeyboardCues)
            format |= DT_HIDEPREFIX;

        dc.DrawText(szText, -1, &lpTBCustomDraw->nmcd.rc, format);

        if (hFont != NULL)
            dc.SelectFont(hFontOld);
    }

    // Implementation

    bool CreateInternalImageList(int cImages)
    {
        UINT uFlags = (m_bAlphaImages ? ILC_COLOR32 : ILC_COLOR24) | ILC_MASK;
        m_hImageList = ::ImageList_Create(m_szBitmap.cx, m_szBitmap.cy, uFlags, cImages, 1);
        ATLASSERT(m_hImageList != NULL);
        return (m_hImageList != NULL);
    }
};

class CCoolMenuBar : public CCoolMenuBarImpl<CCoolMenuBar>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_CoolMenuBar"), GetWndClassName())
};

} // namespace WTL

#endif // __COOLMENUBAR_H__
