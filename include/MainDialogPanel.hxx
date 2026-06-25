//
// MainDialogPanel.hxx
//

#ifndef MAINDIALOGPANEL_HXX
#define MAINDIALOGPANEL_HXX

class ConfigFile;
class wxTextCtrl;
class wxButton;
class wxListBox;

class CMainDialogPanel : public wxPanel 
{
    ConfigFile *m_confFile;
    public:
        CMainDialogPanel(wxWindow* parent, ConfigFile *cfgFile = nullptr);

    private:
        wxTextCtrl* m_txtLlamaBin;
        wxButton* m_btnOpenFolder;
        wxListBox* m_listModels;

        void OnOpenFolder(wxCommandEvent& event);
};

#endif // MAINDIALOGPANEL_HXX