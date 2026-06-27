//
// BuildDialogLin.hxx
//

#ifndef BUILDDIALOGLIN_HXX
#define BUILDDIALOGLIN_HXX

#include "prc.hxx"

class BuildDialogLin : public wxDialog
{
public:
    BuildDialogLin(wxWindow* parent);
    ~BuildDialogLin() = default;

private:
    wxString CMakePath();
    void OnCheckCMake(wxCommandEvent& event);
    void OnOpenLlamaSource(wxCommandEvent& event);
    void OnDownloadLlama(wxCommandEvent& event);

    wxButton* m_sourceButton;
    wxButton* m_downloadButton;
    wxButton* m_checkButton;
    wxTextCtrl* m_llamaSource;
    wxTextCtrl* m_cmakePathText;
};

#endif // BUILDDIALOGLIN_HXX
