//
// MyDialog.cxx
//

#include "prc.hxx"
#include "MainDialogMenu.hxx"
#include "ConfigFile.hxx"
#include "ConfigDlg.hxx"
#include "MainDialogPanel.hxx"

#include "MainDialog.hxx"
#include "MainDialogStatusBar.hxx"

#include "../resources/app.xpm"



CMainDialog::CMainDialog()
    : wxFrame(NULL, wxID_ANY, g_APP_NAME, wxDefaultPosition, wxSize(700, 500), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
    , m_ConfFile(std::make_unique<ConfigFile>())
{
    TrFu;
    
    SetIcon(wxIcon(app_xpm));
    SetFont(wxFont(wxFontInfo(g_DefaultTxtSize).FaceName(g_DefaultTxtFontName)));
    
    m_menuBar = new CMainDialogMenu();
    SetMenuBar(m_menuBar);
    
    // wxFrame automatically handles sizing a single child wxPanel to fill its client area.
    new CMainDialogPanel(this);

    // create and register status bar so wxFrame lays it out at the bottom
    m_statusBar = new CMainDialogStatusBar(this);
    SetStatusBar(m_statusBar);

    Bind(wxEVT_MENU, &CMainDialog::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &CMainDialog::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &CMainDialog::OnSettings, this, ID_SETTINGS);
    Bind(wxEVT_CLOSE_WINDOW, &CMainDialog::OnClose, this);
}

void CMainDialog::OnExit(wxCommandEvent& event) 
{
    Close(true);
}

void CMainDialog::OnAbout(wxCommandEvent& event) 
{
     wxAboutDialogInfo aboutInfo;
    
    aboutInfo.SetName(g_APP_NAME);
    aboutInfo.SetVersion("1.0.0");
    aboutInfo.SetDescription("Light AI Assistant Utility.\nThis program use llama.cpp.\nBuilt with wxWidgets.\nIt is free software, no license, no warranty.\nYou use it on own risk ;)\n");
    aboutInfo.SetCopyright("(C) 2026 Mykhaylo Poboynyy");
    // aboutInfo.SetWebSite("https://your-website.com");
    aboutInfo.SetIcon(wxIcon(app_xpm));
    wxAboutBox(aboutInfo, this);
}

void CMainDialog::OnSettings(wxCommandEvent& event) 
{
    ConfigDlg dlg(this);
    dlg.ShowModal();
}

void CMainDialog::OnClose(wxCloseEvent& event) 
{
    if (wxTheApp && wxTheApp->IsMainLoopRunning()) {
        wxTheApp->ExitMainLoop();
    }
    Destroy();
}
