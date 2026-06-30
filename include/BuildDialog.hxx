//
// BuildDialogLin.hxx
//

#ifndef BUILDDIALOG_HXX
#define BUILDDIALOG_HXX

#include "prc.hxx"

class BuildDialog : public wxDialog
{
public:
    BuildDialog(wxWindow* parent);
    ~BuildDialog() = default;

private:
    wxString CMakePath();
    void OnCheckCMake(wxCommandEvent& event);
    void OnOpenLlamaSource(wxCommandEvent& event);
    void OnDownloadLlama(wxCommandEvent& event);
    void OnUnzipLlama(wxCommandEvent& event);

    wxButton* m_sourceButton;
    wxButton* m_downloadButton;
    wxButton* m_checkButton;
    wxButton* m_unzipButton;
    wxTextCtrl* m_llamaSource;
    wxTextCtrl* m_cmakePathText;
};

#endif // BUILDDIALOG_HXX
