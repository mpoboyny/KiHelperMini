//
// DialogSuggestion.cxx
//

#include "prc.hxx"
#include "DialogSuggestion.hxx"
#include "ConfigFile.hxx"
#include "../resources/app.xpm"
#include "../resources/conf_model.xpm"

namespace
{
    wxString TrimValue(const wxString& value)
    {
        wxString res = value;
        res.Trim(true);
        res.Trim(false);
        return res;
    }

    wxString JoinStrings(const wxArrayString& arr, const wxString& sep)
    {
        wxString out;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i > 0)
                out += sep;
            out += arr[i];
        }
        return out;
    }

    wxString ToGiBString(unsigned long long bytes)
    {
        const double gib = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
        return wxString::Format("%.1f GB", gib);
    }

#ifdef _WIN32
    wxString GetCpuInfoText()
    {
        wxString cpuName;
        wxGetEnv("PROCESSOR_IDENTIFIER", &cpuName);
        if (cpuName.IsEmpty())
            cpuName = "Unknown CPU";

        int cores = wxThread::GetCPUCount();
        if (cores < 1)
            cores = 0;

        return wxString::Format("%s (%d cores)", cpuName, cores);
    }

    wxString GetRamInfoText()
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        if (!GlobalMemoryStatusEx(&statex))
            return "Unknown RAM";

        return ToGiBString(statex.ullTotalPhys);
    }

    wxString GetGpuInfoText()
    {
        wxArrayString out;
        wxArrayString err;
        long code = wxExecute("wmic path win32_VideoController get Name /value", out, err, wxEXEC_SYNC);
        if (code == -1)
            return "Unknown GPU";

        wxArrayString gpus;
        for (const auto& lineRaw : out) {
            wxString line = TrimValue(lineRaw);
            if (!line.StartsWith("Name="))
                continue;

            wxString name = TrimValue(line.AfterFirst('='));
            if (!name.IsEmpty() && gpus.Index(name) == wxNOT_FOUND)
                gpus.Add(name);
        }

        if (gpus.IsEmpty())
            return "Unknown GPU";

        return JoinStrings(gpus, "; ");
    }

#elif defined(__gnu_linux__)
    wxString GetCpuInfoText()
    {
        wxString cpuName = "Unknown CPU";
        std::ifstream cpuFile("/proc/cpuinfo");
        if (cpuFile.is_open()) {
            std::string line;
            while (std::getline(cpuFile, line)) {
                const std::string key = "model name";
                if (line.rfind(key, 0) == 0) {
                    std::size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        cpuName = wxString::FromUTF8(line.substr(pos + 1));
                        cpuName = TrimValue(cpuName);
                    }
                    break;
                }
            }
        }

        int cores = wxThread::GetCPUCount();
        if (cores < 1)
            cores = 0;

        return wxString::Format("%s (%d cores)", cpuName, cores);
    }

    wxString GetRamInfoText()
    {
        std::ifstream memFile("/proc/meminfo");
        if (!memFile.is_open())
            return "Unknown RAM";

        std::string line;
        while (std::getline(memFile, line)) {
            const std::string key = "MemTotal:";
            if (line.rfind(key, 0) == 0) {
                std::string num;
                for (char c : line) {
                    if (c >= '0' && c <= '9')
                        num.push_back(c);
                }
                if (!num.empty()) {
                    unsigned long long kb = std::strtoull(num.c_str(), nullptr, 10);
                    return ToGiBString(kb * 1024ULL);
                }
                break;
            }
        }

        return "Unknown RAM";
    }

    wxString GetGpuInfoText()
    {
        wxArrayString out;
        wxArrayString err;
        long code = wxExecute("lspci -mm", out, err, wxEXEC_SYNC);
        if (code == -1)
            return "Unknown GPU";

        wxArrayString gpus;
        for (const auto& raw : out) {
            wxString line = raw;
            if (!(line.Contains("\"VGA compatible controller\"") ||
                  line.Contains("\"3D controller\"") ||
                  line.Contains("\"Display controller\""))) {
                continue;
            }

            wxArrayString quoted;
            bool inQuote = false;
            wxString curr;
            for (wxUniChar ch : line) {
                if (ch == '"') {
                    if (inQuote) {
                        quoted.Add(curr);
                        curr.clear();
                    }
                    inQuote = !inQuote;
                } else if (inQuote) {
                    curr += ch;
                }
            }

            if (quoted.size() >= 3) {
                wxString name = quoted[1] + " " + quoted[2];
                name = TrimValue(name);
                if (!name.IsEmpty() && gpus.Index(name) == wxNOT_FOUND)
                    gpus.Add(name);
            }
        }

        if (gpus.IsEmpty())
            return "Unknown GPU";

        return JoinStrings(gpus, "; ");
    }
#else
    wxString GetCpuInfoText()
    {
        return "Unknown CPU";
    }

    wxString GetRamInfoText()
    {
        return "Unknown RAM";
    }

    wxString GetGpuInfoText()
    {
        return "Unknown GPU";
    }
#endif
}

/*static*/
const wxSize DialogSuggestion::s_defSize = wxSize(860, 980);

DialogSuggestion::DialogSuggestion(wxWindow* parent, const ConfigFile& confFile)
    : wxDialog(parent, wxID_ANY, "Parameter suggestion", wxDefaultPosition, s_defSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    TrFu;
    SetIcon(wxIcon(app_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* modelBox = new wxStaticBoxSizer(wxVERTICAL, this, "Model");
    auto models = confFile.GetModels();
    wxArrayString modelChoices;
    int currentModelIndex = wxNOT_FOUND;
    int index = 0;
    for (const auto& model : models) {
        modelChoices.Add(model.Path);
        if (model.Current) {
            currentModelIndex = index;
        }
        ++index;
    }

    if (modelChoices.IsEmpty()) {
        modelChoices.Add("Current");
        currentModelIndex = 0;
    } else if (currentModelIndex == wxNOT_FOUND) {
        currentModelIndex = 0;
    }

    m_modelCmb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, modelChoices, wxCB_READONLY);
    m_modelCmb->SetSelection(currentModelIndex);

    modelBox->Add(m_modelCmb, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(modelBox, 0, wxALL | wxEXPAND, 12);

    wxStaticBoxSizer* llamaBox = new wxStaticBoxSizer(wxVERTICAL, this, "llama.cpp");

    wxFlexGridSizer* llamaGrid = new wxFlexGridSizer(2, 2, 8, 8);
    llamaGrid->AddGrowableCol(1, 1);

#ifdef _WIN32
    const wxString cliExeName = "llama-cli.exe";
    const wxString serverExeName = "llama-server.exe";
#else
    const wxString cliExeName = "llama-cli";
    const wxString serverExeName = "llama-server";
#endif

    wxString llamaBinPath = confFile.GetLLamaBinPath();
    wxString llamaCliPath;
    wxString llamaServerPath;
    if (!llamaBinPath.IsEmpty()) {
        llamaCliPath = wxFileName(llamaBinPath, cliExeName).GetFullPath();
        llamaServerPath = wxFileName(llamaBinPath, serverExeName).GetFullPath();
    }

    wxStaticText* llamaCliLabel = new wxStaticText(this, wxID_ANY, "llama-cli:");
    wxTextCtrl* llamaCliText = new wxTextCtrl(this, wxID_ANY, llamaCliPath, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    llamaGrid->Add(llamaCliLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    llamaGrid->Add(llamaCliText, 1, wxEXPAND);

    wxStaticText* llamaServerLabel = new wxStaticText(this, wxID_ANY, "llama-server:");
    wxTextCtrl* llamaServerText = new wxTextCtrl(this, wxID_ANY, llamaServerPath, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    llamaGrid->Add(llamaServerLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    llamaGrid->Add(llamaServerText, 1, wxEXPAND);

    llamaBox->Add(llamaGrid, 1, wxALL | wxEXPAND, 10);
    mainSizer->Add(llamaBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* systemBox = new wxStaticBoxSizer(wxVERTICAL, this, "System");

    wxFlexGridSizer* systemGrid = new wxFlexGridSizer(3, 2, 8, 8);
    systemGrid->AddGrowableCol(1, 1);

    wxStaticText* cpuLabel = new wxStaticText(this, wxID_ANY, "CPU:");
    wxTextCtrl* cpuText = new wxTextCtrl(this, wxID_ANY, GetCpuInfoText(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    systemGrid->Add(cpuLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(cpuText, 1, wxEXPAND);

    wxStaticText* ramLabel = new wxStaticText(this, wxID_ANY, "RAM:");
    wxTextCtrl* ramText = new wxTextCtrl(this, wxID_ANY, GetRamInfoText(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    systemGrid->Add(ramLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(ramText, 1, wxEXPAND);

    wxStaticText* gpuLabel = new wxStaticText(this, wxID_ANY, "GPU:");
    wxTextCtrl* gpuText = new wxTextCtrl(this, wxID_ANY, GetGpuInfoText(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    systemGrid->Add(gpuLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(gpuText, 1, wxEXPAND);

    systemBox->Add(systemGrid, 1, wxALL | wxEXPAND, 10);
    mainSizer->Add(systemBox, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* usageBox = new wxStaticBoxSizer(wxVERTICAL, this, "Usage");

    wxCheckBox* chatCheck = new wxCheckBox(this, wxID_ANY, "Chat");
    wxCheckBox* agentCheck = new wxCheckBox(this, wxID_ANY, "Agent");
    wxCheckBox* embeddingCheck = new wxCheckBox(this, wxID_ANY, "Embedding");
    wxCheckBox* autocompleteCheck = new wxCheckBox(this, wxID_ANY, "Autocomplete");

    agentCheck->SetValue(true);

    wxBoxSizer* usageRow = new wxBoxSizer(wxHORIZONTAL);
    usageRow->Add(chatCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(agentCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(embeddingCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(autocompleteCheck, 0, wxALIGN_CENTER_VERTICAL);

    usageBox->Add(usageRow, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(usageBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* outputFormatBox = new wxStaticBoxSizer(wxVERTICAL, this, "Output format");

    wxCheckBox* cliRunParamsCheck = new wxCheckBox(this, wxID_ANY, "cli run parameters");
    wxCheckBox* serverRunParamsCheck = new wxCheckBox(this, wxID_ANY, "server run parameters");
    wxCheckBox* presetServerIniCheck = new wxCheckBox(this, wxID_ANY, "preset server INI file");

    cliRunParamsCheck->SetValue(true);

    wxBoxSizer* outputFormatRow = new wxBoxSizer(wxHORIZONTAL);
    outputFormatRow->Add(cliRunParamsCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    outputFormatRow->Add(serverRunParamsCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    outputFormatRow->Add(presetServerIniCheck, 0, wxALIGN_CENTER_VERTICAL);

    outputFormatBox->Add(outputFormatRow, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(outputFormatBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* resultBox = new wxStaticBoxSizer(wxVERTICAL, this, "Result");
    const int resultHeight = GetCharHeight() * 10;
    m_resultText = new wxRichTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(-1, resultHeight),
        wxVSCROLL | wxHSCROLL | wxRE_READONLY
    );
    m_resultText->SetMinSize(wxSize(-1, resultHeight));
    resultBox->Add(m_resultText, 1, wxALL | wxEXPAND, 10);
    mainSizer->Add(resultBox, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer(1);
    wxButton* doItBtn = new wxButton(this, wxID_ANY, "Do it!");
    doItBtn->SetBitmap(wxBitmap(conf_model));
    btnSizer->Add(doItBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, "Close");
    btnSizer->Add(closeBtn, 0, wxALIGN_CENTER_VERTICAL);
    mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 10);

    Bind(wxEVT_BUTTON, &DialogSuggestion::OnDoItBtn, this, doItBtn->GetId());
    Bind(wxEVT_BUTTON, &DialogSuggestion::OnCloseBtn, this, wxID_CLOSE);

    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    SetMinSize(s_defSize);
    SetSize(s_defSize);
    Layout();
    CentreOnParent();
}

void DialogSuggestion::OnDoItBtn(wxCommandEvent& event)
{
    if (m_resultText) {
        m_resultText->Clear();
        m_resultText->WriteText("Not implemented yet.");
    }
}

void DialogSuggestion::OnCloseBtn(wxCommandEvent& event)
{
    EndModal(wxID_CLOSE);
}
