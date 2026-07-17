//
// MainDialog.hxx
//

#ifndef MAINDIALOG_HXX
#define MAINDIALOG_HXX

class CMainDialogMenu;
class ConfigFile;
class CMainDialogStatusBar;
class CMainDialogPanel;
class CMainRunPanel;

class CMainDialog : public wxFrame 
{
    CMainDialogMenu* m_menuBar;
    CMainDialogStatusBar* m_statusBar;
    CMainDialogPanel* m_mainPanel;
    CMainRunPanel* m_runPanel;
    
    public:
        CMainDialog();
        
    private:
        std::unique_ptr<ConfigFile> m_ConfFile;

        void OnExit(wxCommandEvent& event);
        void OnReset(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnSettings(wxCommandEvent& event);
        void OnBuild(wxCommandEvent& event);
        void OnBuildLlama(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
};

#endif // MAINDIALOG_HXX
