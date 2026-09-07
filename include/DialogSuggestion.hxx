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

    static int ParseLastInteger(const wxString& text);
    static bool LooksUnknownValue(const wxString& text);
    static wxString FormatGiB(uint64_t bytes);
    static wxString FormatBillions(uint64_t value);
    static wxString GetModelDescription(const llama_model* model);
    static wxString FormatRamInfoText(const wxString& name, const wxString& type, int sizeGB, int speedMTs);
    static wxString FormatGpuInfoText(const wxString& name, int vramGB);
    static wxString TrimValue(const wxString& value);
    static INI::CpuInfo GetCpuInfoText();
    static INI::RamInfo GetRamInfoText();
    static unsigned long long GetGpuVramBytes();
    static INI::GpuInfo GetGpuInfoText();

    void AddStep(const wxString& text);
    void AddInfo(const wxString& text);
    void AddWarning(const wxString& text);
    void AddError(const wxString& text);
    void AddSettingsSummary();
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
