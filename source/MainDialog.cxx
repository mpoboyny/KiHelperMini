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
#include "BuildLlama.hxx"
#include "BuildDialog.hxx"

#include "../resources/app.xpm"

CMainDialog::CMainDialog()
    : wxFrame(NULL, wxID_ANY, g_APP_NAME, wxDefaultPosition, wxSize(900, 700), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
    , m_ConfFile(std::make_unique<ConfigFile>())
{
    TrFu;
    
    SetIcon(wxIcon(app_xpm));
    SetFont(wxFont(wxFontInfo(g_DefaultTxtSize).FaceName(g_DefaultTxtFontName)));
    
    m_menuBar = new CMainDialogMenu();
    SetMenuBar(m_menuBar);
    
    // wxFrame automatically handles sizing a single child wxPanel to fill its client area.
    new CMainDialogPanel(this, m_ConfFile.get());

    // create and register status bar so wxFrame lays it out at the bottom
    m_statusBar = new CMainDialogStatusBar(this);
    SetStatusBar(m_statusBar);

    Bind(wxEVT_MENU, &CMainDialog::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &CMainDialog::OnReset, this, ID_RESET);
    Bind(wxEVT_MENU, &CMainDialog::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &CMainDialog::OnSettings, this, ID_SETTINGS);
    Bind(wxEVT_MENU, &CMainDialog::OnBuild, this, ID_BUILD);
    Bind(wxEVT_MENU, &CMainDialog::OnBuildLlama, this, ID_BUILD_LLAMA);
    Bind(wxEVT_CLOSE_WINDOW, &CMainDialog::OnClose, this);

    wxString msg;
    if (m_ConfFile->HaveError(msg))
        m_statusBar->ShowMessage(GuiStatus::GuiStatus_Erro, msg);
    else
        m_statusBar->ShowReady();
}

void CMainDialog::OnExit(wxCommandEvent &event)
{
    Close(true);
}

void CMainDialog::OnReset(wxCommandEvent &event)
{
    // ask user for confirmation before deleting current config
    wxString ask = wxString::Format("Are you sure you want to delete current\n%s\nfile and overwrite with default?", g_ConfFile);
    int answer = ShowGenericMessageBox(ask, "Confirm Reset", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this);
    TrVar(answer);
    if (answer != wxID_YES) {
        return;
    }

    if (wxFileName::FileExists(g_ConfFile)) {
        wxRemoveFile(g_ConfFile);
    }

    wxExecute(wxStandardPaths::Get().GetExecutablePath());
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
    TrFu;
    ConfigDlg* dlg = new ConfigDlg(this);
    if (dlg->ShowModal() == wxID_OK) {
        if (dlg->IsModified()) {
            dlg->SaveConfig();
            wxExecute(wxStandardPaths::Get().GetExecutablePath());
            dlg->Destroy();
            Close(true);
            return;
        }
    }
    dlg->Destroy();
}

void CMainDialog::OnBuild(wxCommandEvent& event)
{
    ShowGenericMessageBox("Build is not implemented yet.", "Build", wxOK | wxICON_INFORMATION, this);
}

void CMainDialog::OnBuildLlama(wxCommandEvent& event)
{
#if defined(_WIN32) || defined(__gnu_linux__)
    BuildDialog winDlg(this);
    winDlg.ShowModal();
    winDlg.Destroy();
#else
    BuildLlama dlg(this);
    dlg.ShowModal();
    dlg.Destroy();
#endif
}

void CMainDialog::OnClose(wxCloseEvent& event) 
{
    if (wxTheApp && wxTheApp->IsMainLoopRunning()) {
        wxTheApp->ExitMainLoop();
    }
    Destroy();
}
