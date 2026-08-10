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
    wxButton *m_buttChatDoIt;
    wxButton *m_buttChatShowHelp;
    wxButton *m_buttChatCopyScript;
    wxWindow *m_parentWindow;
    wxString m_Modell;

    void OnSelectFile(wxCommandEvent& event);
    void OnRunChat(wxCommandEvent& event);
    void OnRunChatHelp(wxCommandEvent& event);
    void OnCopyChatScript(wxCommandEvent& event);

public:
    CMainRunChatPanel(wxWindow* parent, const ConfigFile &confFile);

};

#endif // MAINRUNCHATPANEL_HXX