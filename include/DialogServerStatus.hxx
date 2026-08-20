//
// DialogServerStatus.hxx
//

#ifndef DIALOGSERVERSTATUS_HXX
#define DIALOGSERVERSTATUS_HXX

class DialogServerStatus : public wxDialog
{
    static const wxSize s_defSize;

    wxTextCtrl* m_textCtrl;
    wxButton* m_killBtn;
    wxButton* m_closeBtn;
    long m_serverPid = -1;

    long FindServerPid();
    wxArrayString GetLastLogLines(size_t maxLines);
    void UpdateStatus();

    void OnKill(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

public:
    DialogServerStatus(wxWindow* parent);
    ~DialogServerStatus() override = default;
};

#endif // DIALOGSERVERSTATUS_HXX
