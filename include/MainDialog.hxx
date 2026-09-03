//
// MainDialog.hxx
//

#ifndef MAINDIALOG_HXX
#define MAINDIALOG_HXX

class CMainDialogMenu;
class ConfigFile;
class CMainDialogStatusBar;
class CMainDialogPanel;
class CMainRunChatPanel;
class CMainRunServerPanel;

class CMainDialog : public wxFrame 
{
    CMainDialogMenu* m_menuBar;
    CMainDialogStatusBar* m_statusBar;
    CMainDialogPanel* m_mainPanel;
    CMainRunChatPanel* m_runPanel;
    CMainRunServerPanel* m_runServerPanel;
    
    static const wxSize s_defMinSize;

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
        void OnParameterSuggestion(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
};

#endif // MAINDIALOG_HXX
