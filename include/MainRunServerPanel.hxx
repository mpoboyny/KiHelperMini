//
// MainRunServerPanel.hxx
//

#ifndef MAINRUNSERVERPANEL_HXX
#define MAINRUNSERVERPANEL_HXX

class ConfigFile;

class CMainRunServerPanel : public wxPanel
{
    static const wxString s_ServerFileName;
    wxTextCtrl *m_textServerFilePath;
    wxTextCtrl *m_textServerParams;
    wxButton *m_buttServerDoIt;
    wxCheckBox *m_checkBox;
    wxButton *m_buttServerShowHelp;
    wxButton *m_buttServerStatus;
    wxButton *m_buttServerCopyScript;
    wxWindow *m_parentWindow;
    wxString m_Modell;
    const ConfigFile &m_ConfFile;

    void OnSelectFile(wxCommandEvent& event);
    void OnRunServer(wxCommandEvent& event);
    void OnRunServerHelp(wxCommandEvent& event);
    void OnShowServerStatus(wxCommandEvent& event);
    void OnCopyServerScript(wxCommandEvent& event);

public:
    CMainRunServerPanel(wxWindow* parent, const ConfigFile &confFile);
    bool GetServerUrl(wxString& url) const;
    const ConfigFile &GetConfigFile() const { return m_ConfFile; }
};

#endif // MAINRUNSERVERPANEL_HXX
