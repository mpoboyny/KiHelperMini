//
// MainDialog.hxx
//

#ifndef MAINDIALOG_HXX
#define MAINDIALOG_HXX

class CMainDialogMenu;
class ConfigFile;
class CMainDialogStatusBar;

class CMainDialog : public wxFrame 
{
    CMainDialogMenu* m_menuBar;
    CMainDialogStatusBar* m_statusBar;
    
    public:
        CMainDialog();
        
    private:
        std::unique_ptr<ConfigFile> m_ConfFile;

        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnSettings(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
};

#endif // MAINDIALOG_HXX
