//
// BuildDialog.hxx
//

#ifndef BUILDDIALOG_HXX
#define BUILDDIALOG_HXX

class BuildDialog : public wxFrame
{

public:
    BuildDialog(wxWindow* parent);
    ~BuildDialog() = default;

    int ShowModalLike();

private:
    wxString CMakePath();
    void OnCheckCMake(wxCommandEvent& event);
    void OnCheckGcc(wxCommandEvent& event);
    void OnBuildCMake(wxCommandEvent& event);
    void OnBuildGcc(wxCommandEvent& event);
    void OnOpenLlamaSource(wxCommandEvent& event);
    void OnShowFiles(wxCommandEvent& event);
    void OnDownloadLlama(wxCommandEvent& event);
    void OnUnzipLlama(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    wxButton* m_checkCmakeButton;
    wxButton* m_checkGccButton;
    wxButton* m_buildCmakeButton;
    wxButton* m_buildGccButton;
    wxRadioButton* m_withCudaRadio;
    wxRadioButton* m_noCudaRadio;
    wxTextCtrl* m_llamaSource;
    wxTextCtrl* m_cmakePathText;
    wxTextCtrl* m_gppInfoText;
    wxWindow* m_parent{nullptr};
    wxEventLoopBase* m_eventLoop{nullptr};
};

#endif // BUILDDIALOG_HXX
