//
// DialogSuggestion.hxx
//

#ifndef DIALOGSUGGESTION_HXX
#define DIALOGSUGGESTION_HXX

class ConfigFile;
struct llama_model;
namespace INI { struct CpuInfo; struct GpuInfo; struct RamInfo; }

class DialogSuggestion : public wxDialog
{
    struct SuggestedParameter
    {
        wxString Name;
        wxString Value;
    };
    typedef std::list<SuggestedParameter> SuggestedParameterList;

    static const wxSize s_defSize;

    int m_CheckStep;

    wxComboBox* m_modelCmb;
    wxComboBox* m_cpuCmb;
    wxComboBox* m_ramCmb;
    wxComboBox* m_gpuCmb;
    wxCheckBox* m_nativeDriverCheck;
    wxCheckBox* m_chatCheck;
    wxCheckBox* m_agentCheck;
    wxCheckBox* m_embeddingCheck;
    wxCheckBox* m_autocompleteCheck;
    wxRadioButton* m_optimizePerformanceRadio;
    wxRadioButton* m_optimizeContextSizeRadio;
    wxComboBox* m_optimizeContextSizeText;
    wxCheckBox* m_cliRunParamsCheck;
    wxCheckBox* m_serverRunParamsCheck;
    wxRichTextCtrl* m_resultText;
    wxFont m_resultTextFont;
    wxButton* m_copyBtn;
    wxButton* m_setRunParamsAsDefaultBtn;
    wxTextCtrl* m_llamaCliText;
    wxTextCtrl* m_llamaServerText;
    wxString m_llamaCliHelp;
    wxString m_llamaServerHelp;
    SuggestedParameterList m_cliSuggestedParameters;
    SuggestedParameterList m_serverSuggestedParameters;

    static int ParseLastInteger(const wxString& text);
    static bool LooksUnknownValue(const wxString& text);
    static wxString FormatGiB(uint64_t bytes);
    static wxString FormatBillions(uint64_t value);
    static wxString GetModelDescription(const llama_model* model);
    static wxString FormatRamInfoText(const wxString& name, const wxString& type, int sizeGB, int speedMTs);
    static wxString FormatGpuInfoText(const wxString& name, int vramGB);
    static wxString TrimValue(const wxString& value);
    static wxString GetFullExecutablePath(const wxString& path);
    static bool IsParameterSupported(const wxString& helpText, const wxString& parameter);
    static INI::CpuInfo GetCpuInfoText();
    static INI::RamInfo GetRamInfoText();
    static unsigned long long GetGpuVramBytes();
    static INI::GpuInfo GetGpuInfoText();

    void AddStep(const wxString& text);
    void AddInfo(const wxString& text);
    void AddWarning(const wxString& text);
    void AddError(const wxString& text);
    void AddSettingsSummary();
    bool LoadLlamaHelp(wxTextCtrl* textCtrl, const wxString& executableName, wxString& helpText);
    bool LoadModel(const wxString& modelPath, llama_model*& currentModel);
    bool CreateSuggestion(const llama_model* currentModel);
    void OnCopyToClipboard(wxCommandEvent& event);
    void OnSetRunParamsAsDefaultBtn(wxCommandEvent& event);
    void OnSelectLlamaCliFile(wxCommandEvent& event);
    void OnSelectLlamaServerFile(wxCommandEvent& event);
    void OnOptimizeContextSizeRadio(wxCommandEvent& event);
    void SelectLlamaFile(wxTextCtrl* textCtrl, const wxString& executableName);

    bool LoadModelInfo(const ConfigFile& confFile);
    
    void OnDoItBtn(wxCommandEvent& event);
    void OnCloseBtn(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);

public:
    DialogSuggestion(wxWindow* parent, const ConfigFile& confFile);
    ~DialogSuggestion() override = default;
};

#endif // DIALOGSUGGESTION_HXX
