//
// ConfigDlg.hxx
//

#ifndef CONFIGDLG_HXX
#define CONFIGDLG_HXX

class ConfigDlg : public wxDialog
{
public:
    ConfigDlg(wxWindow* parent);
    bool IsModified() const;
    void SaveConfig();
private:
    wxStyledTextCtrl* m_textCtrl; // Changed from wxTextCtrl
    wxString m_originalContent;
    void LoadXmlContent();
    void SetupXmlHighlighting();

    void OnShowConfFolder(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif
