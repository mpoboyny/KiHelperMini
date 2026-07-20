//
// MainRunChatPanel.hxx
//

#ifndef MAINRUNCHATPANEL_HXX
#define MAINRUNCHATPANEL_HXX

class CMainRunChatPanel : public wxPanel
{
    static const wxString s_ChatFileName;
    wxTextCtrl *m_textFilePath;
    void OnRunSample(wxCommandEvent& event);
    void OnSelectFile(wxCommandEvent& event);

public:
    CMainRunChatPanel(wxWindow* parent, wxString defDirChatFileDir);

};

#endif // MAINRUNCHATPANEL_HXX