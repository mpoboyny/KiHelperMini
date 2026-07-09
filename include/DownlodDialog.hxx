//
// DownlodDialog.hxx
//

#ifndef DOWNLODDIALOG_HXX
#define DOWNLODDIALOG_HXX

class DownlodDialog : public wxDialog
{
    static const wxString s_defLink;
    static const wxSize s_defSize;
    
    bool m_downloadResult = false;
    wxString m_savedFile;

    wxButton *m_downloadButt;
public:
    static const wxString s_defSaveDir;

    DownlodDialog(wxWindow* parent);
    ~DownlodDialog() = default;

    inline  bool GetDownloadResult() { return m_downloadResult;}
    inline  wxString GetSavedFile() { return m_savedFile;}

private:
    void OnDownload(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    wxTextCtrl* m_fromText;
    wxTextCtrl* m_saveInText;
};

#endif // DOWNLODDIALOG_HXX
