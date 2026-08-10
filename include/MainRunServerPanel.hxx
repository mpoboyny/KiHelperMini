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
    wxButton *m_buttServerShowHelp;
    wxWindow *m_parentWindow;
    wxString m_Modell;

    void OnSelectFile(wxCommandEvent& event);
    void OnRunServer(wxCommandEvent& event);
    void OnRunServerHelp(wxCommandEvent& event);

public:
    CMainRunServerPanel(wxWindow* parent, const ConfigFile &confFile);
};

#endif // MAINRUNSERVERPANEL_HXX
