//
// DialogSuggestion.hxx
//

#ifndef DIALOGSUGGESTION_HXX
#define DIALOGSUGGESTION_HXX

class ConfigFile;
struct llama_model;

class DialogSuggestion : public wxDialog
{
    static const wxSize s_defSize;

    int m_CheckStep;

    wxComboBox* m_modelCmb;
    wxComboBox* m_cpuCmb;
    wxComboBox* m_ramCmb;
    wxComboBox* m_gpuCmb;
    wxCheckBox* m_chatCheck;
    wxCheckBox* m_agentCheck;
    wxCheckBox* m_embeddingCheck;
    wxCheckBox* m_autocompleteCheck;
    wxCheckBox* m_cliRunParamsCheck;
    wxCheckBox* m_serverRunParamsCheck;
    wxCheckBox* m_presetServerIniCheck;
    wxRichTextCtrl* m_resultText;
    wxFont m_resultTextFont;

    void AddStep(const wxString& text);
    void AddInfo(const wxString& text);
    void AddWarning(const wxString& text);
    void AddError(const wxString& text);
    bool LoadModel(const wxString& modelPath, llama_model*& currentModel);
    bool CreateSuggestion(const llama_model* currentModel);

    bool LoadModelInfo(const ConfigFile& confFile);
    
    void OnDoItBtn(wxCommandEvent& event);
    void OnCloseBtn(wxCommandEvent& event);

public:
    DialogSuggestion(wxWindow* parent, const ConfigFile& confFile);
    ~DialogSuggestion() override = default;
};

#endif // DIALOGSUGGESTION_HXX
