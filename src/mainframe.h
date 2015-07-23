#pragma once

class CMainFrame
{
private:
    class Impl;
    std::unique_ptr<Impl> m_pimpl;

public:
    CMainFrame();
    ~CMainFrame();

    HWND Create();
    ATL::CWindow ToCWindow() const;

private:
    CMainFrame(const CMainFrame &);
    CMainFrame &operator=(const CMainFrame &);
};
