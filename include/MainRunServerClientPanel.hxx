//
// MainRunServerClientPanel.hxx
//

#ifndef MAINRUNSERVERCLIENTPANEL_HXX
#define MAINRUNSERVERCLIENTPANEL_HXX

class CMainRunServerPanel;

class CMainRunServerClientPanel : public wxPanel
{
    const CMainRunServerPanel* m_serverPanel;
    wxRadioButton* m_webRadioButton;
    wxRadioButton* m_openCodeRadioButton;

    void OnRun(wxCommandEvent& event);
    void RunWeb();
    void RunOpenCode();

public:
    CMainRunServerClientPanel(wxWindow* parent, const CMainRunServerPanel* serverPanel);
};

#endif // MAINRUNSERVERCLIENTPANEL_HXX
