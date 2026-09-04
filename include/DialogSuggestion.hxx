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
    wxComboBox* m_cpuCmb;
    wxComboBox* m_ramCmb;
    wxComboBox* m_gpuCmb;
    wxRichTextCtrl* m_resultText;

    void OnDoItBtn(wxCommandEvent& event);
    void OnCloseBtn(wxCommandEvent& event);

public:
    DialogSuggestion(wxWindow* parent, const ConfigFile& confFile);
    ~DialogSuggestion() override = default;
};

#endif // DIALOGSUGGESTION_HXX
