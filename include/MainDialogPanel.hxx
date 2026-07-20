//
// MainDialogPanel.hxx
//

#ifndef MAINDIALOGPANEL_HXX
#define MAINDIALOGPANEL_HXX

class ConfigFile;
class wxTextCtrl;
class wxButton;
class wxListCtrl;

class CMainDialogPanel : public wxPanel 
{
    ConfigFile *m_confFile;
    public:
        CMainDialogPanel(wxWindow* parent, ConfigFile *cfgFile = nullptr);

        inline wxString GetLlamaBinPath() const { return m_txtLlamaBin->GetValue(); }

    private:
        wxTextCtrl* m_txtLlamaBin;
        wxButton* m_btnOpenFolder;
        wxListCtrl* m_listModels;

        void OnOpenFolder(wxCommandEvent& event);
};

#endif // MAINDIALOGPANEL_HXX