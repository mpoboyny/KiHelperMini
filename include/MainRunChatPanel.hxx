//
// MainRunChatPanel.hxx
//

#ifndef MAINRUNCHATPANEL_HXX
#define MAINRUNCHATPANEL_HXX

class ConfigFile;

class CMainRunChatPanel : public wxPanel
{
    static const wxString s_ChatFileName;
    wxTextCtrl *m_textChatFilePath;
    wxTextCtrl *m_textChatParams;
    void OnRunSample(wxCommandEvent& event);
    void OnSelectFile(wxCommandEvent& event);

public:
    CMainRunChatPanel(wxWindow* parent, const ConfigFile &confFile);

};

#endif // MAINRUNCHATPANEL_HXX