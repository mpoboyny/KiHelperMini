//
// DownlodDialog.hxx
//

#ifndef DOWNLODDIALOG_HXX
#define DOWNLODDIALOG_HXX

#include "prc.hxx"

class DownlodDialog : public wxDialog
{
    static const wxString s_defLink;
    static const wxSize s_defSize;
    static const wxString s_defSaveDir;
public:
    DownlodDialog(wxWindow* parent);
    ~DownlodDialog() = default;

private:
    void OnDonload(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    wxTextCtrl* m_fromText;
    wxTextCtrl* m_saveInText;
};

#endif // DOWNLODDIALOG_HXX
