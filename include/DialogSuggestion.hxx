//
// DialogSuggestion.hxx
//

#ifndef DIALOGSUGGESTION_HXX
#define DIALOGSUGGESTION_HXX

class ConfigFile;

class DialogSuggestion : public wxDialog
{
    static const wxSize s_defSize;

    wxComboBox* m_modelCmb;
    wxRichTextCtrl* m_resultText;

    void OnDoItBtn(wxCommandEvent& event);
    void OnCloseBtn(wxCommandEvent& event);

public:
    DialogSuggestion(wxWindow* parent, const ConfigFile& confFile);
    ~DialogSuggestion() override = default;
};

#endif // DIALOGSUGGESTION_HXX
