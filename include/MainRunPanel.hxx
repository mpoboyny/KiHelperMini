//
// MainRunPanel.hxx
//

#ifndef MAINRUNPANEL_HXX
#define MAINRUNPANEL_HXX

class MainRunToolBar;

class CMainRunPanel : public wxPanel
{
    MainRunToolBar* m_toolBar;

public:
    CMainRunPanel(wxWindow* parent);

private:
    void OnRunSample(wxCommandEvent& event);
};

#endif // MAINRUNPANEL_HXX