//
// BuildDialogWin.hxx
//

#ifndef BUILDDIALOGWIN_HXX
#define BUILDDIALOGWIN_HXX

#include "prc.hxx"

class BuildDialogWin : public wxDialog
{
public:
    BuildDialogWin(wxWindow* parent);
    ~BuildDialogWin() = default;

private:
    wxString CMakePath();
    void OnFindCMake(wxCommandEvent& event);
    void OnCheckCMake(wxCommandEvent& event);
    void OnOpenLlamaSource(wxCommandEvent& event);

    wxButton* m_sourceButton;
    wxButton* m_checkButton;
    wxTextCtrl* m_llamaSource;
    wxTextCtrl* m_cmakePathText;
};

#endif // BUILDDIALOGWIN_HXX
