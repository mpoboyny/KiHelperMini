//
// DialogPresetIni.hxx
//

#ifndef DIALOGPRESETINI_HXX
#define DIALOGPRESETINI_HXX

class DialogPresetIni : public wxDialog
{
    wxStyledTextCtrl* m_textCtrl;
    wxString m_presetIni;

    void SetupIniHighlighting();
    void OnCopyToClipboard(wxCommandEvent& event);

public:
    DialogPresetIni(wxWindow* parent, const wxString& presetIni);
};

#endif // DIALOGPRESETINI_HXX
