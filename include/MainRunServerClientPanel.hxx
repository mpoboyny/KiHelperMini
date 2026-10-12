//
// MainRunServerClientPanel.hxx
//

#ifndef MAINRUNSERVERCLIENTPANEL_HXX
#define MAINRUNSERVERCLIENTPANEL_HXX

class CMainRunServerPanel;

class CMainRunServerClientPanel : public wxPanel
{
    const CMainRunServerPanel* m_serverPanel;

    void OnRun(wxCommandEvent& event);

public:
    CMainRunServerClientPanel(wxWindow* parent, const CMainRunServerPanel* serverPanel);
};

#endif // MAINRUNSERVERCLIENTPANEL_HXX
