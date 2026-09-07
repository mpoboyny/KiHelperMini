//
// DialogSuggestion.cxx
//

#include "prc.hxx"
#include "DialogSuggestion.hxx"
#include "ConfigFile.hxx"
#include "IniFile.hxx"
#include "ScopeGuard.hxx"
#include "llama.h"
#include "../resources/app.xpm"
#include "../resources/conf_model.xpm"

int DialogSuggestion::ParseLastInteger(const wxString& text)
    {
        int value = 0;
        bool found = false;
        wxString current;

        for (wxUniChar ch : text) {
            if (wxIsdigit(ch)) {
                current += ch;
            } else if (!current.IsEmpty()) {
                long tmp = 0;
                if (current.ToLong(&tmp)) {
                    value = static_cast<int>(tmp);
                    found = true;
                }
                current.clear();
            }
        }

        if (!current.IsEmpty()) {
            long tmp = 0;
            if (current.ToLong(&tmp)) {
                value = static_cast<int>(tmp);
                found = true;
            }
        }

        return found ? value : 0;
    }

bool DialogSuggestion::LooksUnknownValue(const wxString& text)
    {
        return text.Lower().Contains("unknown");
    }

wxString DialogSuggestion::FormatGiB(uint64_t bytes)
    {
        const double gib = 1024.0 * 1024.0 * 1024.0;
        return wxString::Format("%.2f GiB", static_cast<double>(bytes) / gib);
    }

wxString DialogSuggestion::FormatBillions(uint64_t value)
    {
        return wxString::Format("%.2f B", static_cast<double>(value) / 1000000000.0);
    }

wxString DialogSuggestion::GetModelDescription(const llama_model* model)
    {
        char buf[256] = {0};
        if (llama_model_desc(model, buf, sizeof(buf)) > 0)
            return wxString::FromUTF8(buf);
        return "Unknown model";
    }

wxString DialogSuggestion::FormatRamInfoText(const wxString& name, const wxString& type, int sizeGB, int speedMTs)
    {
        const wxString safeName = name.IsEmpty() ? "Unknown" : name;
        const wxString safeType = type.IsEmpty() ? "Unknown" : type;

        wxString result = safeName;
        if (sizeGB > 0)
            result += wxString::Format(" - %d GB", sizeGB);
        else
            result += " - Unknown";

        if (safeType != "Unknown")
            result += " - " + safeType;
        if (speedMTs > 0)
            result += wxString::Format(" (%d MT/s)", speedMTs);
        return result;
    }

wxString DialogSuggestion::FormatGpuInfoText(const wxString& name, int vramGB)
    {
        if (vramGB > 0)
            return wxString::Format("%s (%d GB VRAM)", name, vramGB);
        return name;
    }

wxString DialogSuggestion::TrimValue(const wxString& value)
    {
        wxString res = value;
        res.Trim(true);
        res.Trim(false);
        return res;
    }

#ifdef _WIN32
INI::CpuInfo DialogSuggestion::GetCpuInfoText()
    {
        INI::CpuInfo cpuInfo;
        wxString cpuName;
        wxGetEnv("PROCESSOR_IDENTIFIER", &cpuName);
        if (cpuName.IsEmpty())
            cpuName = "Unknown CPU";

        int cores = wxThread::GetCPUCount();
        if (cores < 1)
            cores = 0;

        cpuInfo.Name = cpuName;
        cpuInfo.Cores = cores;
        return cpuInfo;
    }

INI::RamInfo DialogSuggestion::GetRamInfoText()
    {
        INI::RamInfo ramInfo;
        ramInfo.Name = "Unknown";
        ramInfo.Type = "Unknown";
        ramInfo.SizeGB = 0;
        ramInfo.SpeedMTs = 0;

        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        if (!GlobalMemoryStatusEx(&statex))
            return ramInfo;

        const unsigned long long gib = 1024ULL * 1024ULL * 1024ULL;
        ramInfo.Name = "Installed RAM";
        ramInfo.SizeGB = static_cast<int>((statex.ullTotalPhys + (gib / 2ULL)) / gib);
        return ramInfo;
    }

INI::GpuInfo DialogSuggestion::GetGpuInfoText()
    {
        INI::GpuInfo gpuInfo;
        gpuInfo.Name = "Unknown GPU";
        gpuInfo.VramGB = 0;

        wxArrayString out;
        wxArrayString err;
        long code = wxExecute("wmic path win32_VideoController get Name /value", out, err, wxEXEC_SYNC);
        if (code == -1)
            return gpuInfo;

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
            return gpuInfo;

        gpuInfo.Name = JoinStrings(gpus, "; ");
        return gpuInfo;
    }

#elif defined(__gnu_linux__)
INI::CpuInfo DialogSuggestion::GetCpuInfoText()
    {
        INI::CpuInfo cpuInfo;
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

        cpuInfo.Name = cpuName;
        cpuInfo.Cores = cores;
        return cpuInfo;
    }

INI::RamInfo DialogSuggestion::GetRamInfoText()
{
    INI::RamInfo ramInfo;
    ramInfo.Name = "Unknown";
    ramInfo.Type = "Unknown";
    ramInfo.SizeGB = 0;
    ramInfo.SpeedMTs = 0;

    unsigned long long totalRamKB = 0;
    wxString ramType = "";

    // 1. Nutzbaren RAM aus /proc/meminfo lesen
    wxTextFile file;
    if (file.Open("/proc/meminfo"))
    {
        for (wxString line = file.GetFirstLine(); !file.Eof(); line = file.GetNextLine())
        {
            if (line.StartsWith("MemTotal:"))
            {
                wxStringTokenizer tokenizer(line, " \t");
                tokenizer.GetNextToken(); 
                wxString valueStr = tokenizer.GetNextToken();
                valueStr.ToULongLong(&totalRamKB);
                break;
            }
        }
        file.Close();
    }

    if (totalRamKB == 0) return ramInfo;

    // Umrechnung in echte GB
    double reportedGB = (double)totalRamKB / (1024.0 * 1024.0);
    
    // HARDWARE-ANPASSUNG: Linux "unterschlägt" immer etwas RAM für den Kernel/BIOS.
    // Wir runden mathematisch sinnvoll auf die nächste logische Hardware-Größe auf.
    int hardwareGB = 0;
    if (reportedGB > 48.0 && reportedGB <= 64.0)       hardwareGB = 64;
    else if (reportedGB > 24.0 && reportedGB <= 32.0)  hardwareGB = 32;
    else if (reportedGB > 12.0 && reportedGB <= 16.0)  hardwareGB = 16;
    else if (reportedGB > 6.0 && reportedGB <= 8.0)    hardwareGB = 8;
    else if (reportedGB > 96.0 && reportedGB <= 128.0) hardwareGB = 128;
    else {
        // Fallback für krumme Server-Setups oder sehr alte PCs
        hardwareGB = (int)std::ceil(reportedGB);
    }

    // 2. Hardware-Typ aus EDAC sysfs lesen
    wxString basisPfad = "/sys/devices/system/edac/mc/";
    if (wxDir::Exists(basisPfad))
    {
        wxDir dir(basisPfad);
        wxString unterOrdner;
        bool gefunden = dir.GetFirst(&unterOrdner, "mc*", wxDIR_DIRS);
        while (gefunden)
        {
            wxString zielPfad = basisPfad + unterOrdner + "/dimm0/dimm_mem_type";
            if (!wxFileName::FileExists(zielPfad)) {
                zielPfad = basisPfad + unterOrdner + "/rank0/dimm_mem_type";
            }

            if (wxFileName::FileExists(zielPfad)) {
                wxTextFile typeFile;
                if (typeFile.Open(zielPfad)) {
                    ramType = typeFile.GetFirstLine().Trim(true).Trim(false);
                    typeFile.Close();
                    break; 
                }
            }
            gefunden = dir.GetNext(&unterOrdner);
        }
    }

    // 3. Ergebnis formatieren
    // Wenn kein Typ im sysfs gefunden wurde (z.B. in VMs oder bei fehlenden Chipsatz-Treibern),
    // blenden wir die Klammer einfach aus, statt "Unknown Type" anzuzeigen.
    ramInfo.Name = "Installed RAM";
    ramInfo.Type = ramType.IsEmpty() ? "Unknown" : ramType;
    ramInfo.SizeGB = hardwareGB;
    return ramInfo;
}


unsigned long long DialogSuggestion::GetGpuVramBytes()
{
    // === STRATEGIE 1: PROPRIETÄRES NVIDIA TOOL (nvidia-smi) ===
    // Wenn der offizielle Nvidia-Treiber installiert ist, liefert das die sichersten Daten.
    wxArrayString nvOut, nvErr;
    if (wxExecute("nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits", nvOut, nvErr, wxEXEC_SYNC) == 0)
    {
        if (!nvOut.IsEmpty())
        {
            unsigned long long mib = 0;
            if (nvOut[0].Trim(true).Trim(false).ToULongLong(&mib) && mib > 0)
            {
                return mib * 1024 * 1024;
            }
        }
    }

    // === STRATEGIE 2: STANDARD-KERNEL-PFADE (AMD & Intel) ===
    // Funktioniert hervorragend bei AMD (amdgpu) und Intel (i915/xe) Open-Source-Treibern.
    wxString drmPath = "/sys/class/drm/";
    if (wxDir::Exists(drmPath))
    {
        wxDir dir(drmPath);
        wxString subDir;
        bool found = dir.GetFirst(&subDir, "card*", wxDIR_DIRS);
        while (found)
        {
            wxArrayString vramFiles;
            vramFiles.Add(drmPath + subDir + "/device/mem_info_vram_total"); // AMD
            vramFiles.Add(drmPath + subDir + "/device/lmem_total_bytes");     // Intel Arc
            
            for (size_t i = 0; i < vramFiles.GetCount(); ++i)
            {
                if (wxFileName::FileExists(vramFiles[i]))
                {
                    wxTextFile file;
                    if (file.Open(vramFiles[i]))
                    {
                        wxString rawVal = file.GetFirstLine().Trim(true).Trim(false);
                        unsigned long long bytes = 0;
                        if (rawVal.ToULongLong(&bytes) && bytes > 0)
                        {
                            file.Close();
                            return bytes;
                        }
                        file.Close();
                    }
                }
            }
            found = dir.GetNext(&subDir);
        }
    }

    // === STRATEGIE 3: DER ULTIMATIVE FALLBACK (PCI BAR-Größe aus sysfs) ===
    // Funktioniert IMMER, HARDWARE-NATUR und VÖLLIG TREIBERUNABHÄNGIG.
    // Wir schauen uns die physischen PCI-Speicherbereiche der Grafikkarte an.
    wxString pciPath = "/sys/bus/pci/devices/";
    if (wxDir::Exists(pciPath))
    {
        wxDir dir(pciPath);
        wxString deviceDir;
        bool found = dir.GetFirst(&deviceDir, "*", wxDIR_DIRS);
        while (found)
        {
            wxString classPath = pciPath + deviceDir + "/class";
            if (wxFileName::FileExists(classPath))
            {
                wxTextFile classFile;
                if (classFile.Open(classPath))
                {
                    wxString classId = classFile.GetFirstLine().Trim(true).Trim(false);
                    classFile.Close();

                    // 0x030000 = VGA-Controller, 0x030200 = 3D-Controller (z.B. Nvidia Optimus)
                    if (classId.StartsWith("0x0300") || classId.StartsWith("0x0302"))
                    {
                        wxString resourcePath = pciPath + deviceDir + "/resource";
                        if (wxFileName::FileExists(resourcePath))
                        {
                            wxTextFile resFile;
                            if (resFile.Open(resourcePath))
                            {
                                // Die Datei 'resource' listet Start-, Endadresse und Flags der PCI-Speicherbänke.
                                // Der größte Speicherbereich (oft BAR 0 oder BAR 2) ist der VRAM (Frame Buffer).
                                unsigned long long maxBarSize = 0;
                                for (wxString line = resFile.GetFirstLine(); !resFile.Eof(); line = resFile.GetNextLine())
                                {
                                    wxStringTokenizer tokenizer(line, " ");
                                    if (tokenizer.CountTokens() >= 3)
                                    {
                                        unsigned long long start = 0, end = 0;
                                        tokenizer.GetNextToken().ToULongLong(&start, 16); // Hexadezimal
                                        tokenizer.GetNextToken().ToULongLong(&end, 16);
                                        
                                        if (end > start)
                                        {
                                            unsigned long long size = end - start + 1;
                                            // VRAM-Bereiche auf modernen GPUs sind typischerweise mindestens 128MB groß
                                            if (size > maxBarSize && size >= (128 * 1024 * 1024))
                                            {
                                                maxBarSize = size;
                                            }
                                        }
                                    }
                                }
                                resFile.Close();
                                if (maxBarSize > 0) return maxBarSize;
                            }
                        }
                    }
                }
            }
            found = dir.GetNext(&deviceDir);
        }
    }

    return 0; // Wenn absolut alles fehlschlägt, wird nur der Name angezeigt
}


INI::GpuInfo DialogSuggestion::GetGpuInfoText()
{
    INI::GpuInfo gpuInfo;
    gpuInfo.Name = "Unknown GPU";
    gpuInfo.VramGB = 0;

    wxArrayString out;
    wxArrayString err;
    long code = wxExecute("lspci -mm", out, err, wxEXEC_SYNC);
    if (code == -1)
        return gpuInfo;

    wxArrayString gpus;
    for (const auto& raw : out) {
        wxString line = raw;
        if (!(line.Contains("\"VGA compatible controller\"") ||
              line.Contains("\"3D controller\"") ||
              line.Contains("\"Display controller\""))) {
            continue;
        }

        // DEIN ORIGINALES PARSING (Hält bombenfest)
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
            
            // Textbereinigung für eine schönere UI
            name.Replace(" Technologies Inc", "");
            name.Replace(", Inc.", "");
            name.Replace(" Corporation", "");
            
            if (!name.IsEmpty() && gpus.Index(name) == wxNOT_FOUND)
                gpus.Add(name);
        }
    }

    if (gpus.IsEmpty())
        return gpuInfo;

    // Verkettung (falls Dual-GPU wie Intel iGPU + Nvidia dGPU aktiv sind)
    wxString gpuText = "";
    for (size_t i = 0; i < gpus.GetCount(); ++i)
    {
        if (i > 0) gpuText += " & ";
        gpuText += gpus[i];
    }

    gpuInfo.Name = gpuText;

    // VRAM ermitteln
    unsigned long long vramBytes = GetGpuVramBytes();
    if (vramBytes > 0) 
    {
        double vramGB = (double)vramBytes / (1024.0 * 1024.0 * 1024.0);
        gpuInfo.VramGB = (int)(vramGB + 0.1);
    }

    return gpuInfo;
}


#else
INI::CpuInfo DialogSuggestion::GetCpuInfoText()
    {
        INI::CpuInfo cpuInfo;
        cpuInfo.Name = "Unknown CPU";
        cpuInfo.Cores = 0;
        return cpuInfo;
    }

INI::RamInfo DialogSuggestion::GetRamInfoText()
    {
        INI::RamInfo ramInfo;
        ramInfo.Name = "Unknown";
        ramInfo.Type = "Unknown";
        ramInfo.SizeGB = 0;
        ramInfo.SpeedMTs = 0;
        return ramInfo;
    }

INI::GpuInfo DialogSuggestion::GetGpuInfoText()
    {
        INI::GpuInfo gpuInfo;
        gpuInfo.Name = "Unknown GPU";
        gpuInfo.VramGB = 0;
        return gpuInfo;
    }
#endif

/*static*/
const wxSize DialogSuggestion::s_defSize = wxSize(860, 980);

DialogSuggestion::DialogSuggestion(wxWindow* parent, const ConfigFile& confFile)
: wxDialog(parent, wxID_ANY, "Parameter suggestion", wxDefaultPosition, s_defSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
, m_CheckStep(0)
, m_modelCmb(nullptr)
, m_cpuCmb(nullptr)
, m_ramCmb(nullptr)
, m_gpuCmb(nullptr)
, m_chatCheck(nullptr)
, m_agentCheck(nullptr)
, m_embeddingCheck(nullptr)
, m_autocompleteCheck(nullptr)
, m_cliRunParamsCheck(nullptr)
, m_serverRunParamsCheck(nullptr)
, m_presetServerIniCheck(nullptr)
, m_resultText(nullptr)
{
    TrFu;
    SetIcon(wxIcon(app_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* topInfoText = new wxStaticText(
        this,
        wxID_ANY,
        "Current hardware is selected automatically for CPU, RAM, and GPU. Adjust the selections below if needed."
    );
    topInfoText->Wrap(s_defSize.GetWidth() - 60);
    mainSizer->Add(topInfoText, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 12);

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
    const INI::CpuInfo currentCpu = GetCpuInfoText();
    wxArrayString cpuChoices;
    auto cpus = INI::IniFile::Inst().GetCpus(INI::EiniTypeCpu);
    cpus.insert(cpus.begin(), currentCpu);
    for (size_t i = 0; i < cpus.size(); ++i) {
        const auto& cpu = cpus[i];
        wxString cpuText = wxString::Format("%s (%d cores)", cpu.Name, cpu.Cores);
        cpuChoices.Add(cpuText);
    }
    m_cpuCmb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, cpuChoices, wxCB_READONLY);
    m_cpuCmb->SetSelection(0);
    systemGrid->Add(cpuLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(m_cpuCmb, 1, wxEXPAND);

    wxStaticText* ramLabel = new wxStaticText(this, wxID_ANY, "RAM:");
    const INI::RamInfo currentRam = GetRamInfoText();
    wxArrayString ramChoices;
    auto rams = INI::IniFile::Inst().GetRams(INI::EiniTypeRam);
    rams.insert(rams.begin(), currentRam);
    for (size_t i = 0; i < rams.size(); ++i) {
        const auto& ram = rams[i];
        wxString ramText = FormatRamInfoText(ram.Name, ram.Type, ram.SizeGB, ram.SpeedMTs);
        if (ramChoices.Index(ramText) == wxNOT_FOUND)
            ramChoices.Add(ramText);
    }
    m_ramCmb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, ramChoices, wxCB_READONLY);
    m_ramCmb->SetSelection(0);
    systemGrid->Add(ramLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(m_ramCmb, 1, wxEXPAND);

    wxStaticText* gpuLabel = new wxStaticText(this, wxID_ANY, "GPU:");
    wxArrayString gpuChoices;
    const INI::GpuInfo currentGpu = GetGpuInfoText();
    auto gpus = INI::IniFile::Inst().GetGpus(INI::EiniTypeGpu);
    gpus.insert(gpus.begin(), currentGpu);
    for (size_t i = 0; i < gpus.size(); ++i) {
        const auto& gpu = gpus[i];
        wxString gpuText = FormatGpuInfoText(gpu.Name, gpu.VramGB);
        if (gpuChoices.Index(gpuText) == wxNOT_FOUND)
            gpuChoices.Add(gpuText);
    }
    m_gpuCmb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, gpuChoices, wxCB_READONLY);
    m_gpuCmb->SetSelection(0);
    systemGrid->Add(gpuLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
    systemGrid->Add(m_gpuCmb, 1, wxEXPAND);

    systemBox->Add(systemGrid, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(systemBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* usageBox = new wxStaticBoxSizer(wxVERTICAL, this, "Usage");

    m_chatCheck = new wxCheckBox(this, wxID_ANY, "Chat");
    m_agentCheck = new wxCheckBox(this, wxID_ANY, "Agent");
    m_embeddingCheck = new wxCheckBox(this, wxID_ANY, "Embedding");
    m_autocompleteCheck = new wxCheckBox(this, wxID_ANY, "Autocomplete");

    m_agentCheck->SetValue(true);

    wxBoxSizer* usageRow = new wxBoxSizer(wxHORIZONTAL);
    usageRow->Add(m_chatCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(m_agentCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(m_embeddingCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    usageRow->Add(m_autocompleteCheck, 0, wxALIGN_CENTER_VERTICAL);

    usageBox->Add(usageRow, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(usageBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStaticBoxSizer* outputFormatBox = new wxStaticBoxSizer(wxVERTICAL, this, "Output format");

    m_cliRunParamsCheck = new wxCheckBox(this, wxID_ANY, "cli run parameters");
    m_serverRunParamsCheck = new wxCheckBox(this, wxID_ANY, "server run parameters");
    m_presetServerIniCheck = new wxCheckBox(this, wxID_ANY, "preset server INI file");

    m_cliRunParamsCheck->SetValue(true);

    wxBoxSizer* outputFormatRow = new wxBoxSizer(wxHORIZONTAL);
    outputFormatRow->Add(m_cliRunParamsCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    outputFormatRow->Add(m_serverRunParamsCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    outputFormatRow->Add(m_presetServerIniCheck, 0, wxALIGN_CENTER_VERTICAL);

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
    m_resultTextFont = m_resultText->GetFont();
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

void DialogSuggestion::AddStep(const wxString& text)
{
    ++m_CheckStep;
    if (!m_resultText)
        return;

    m_resultText->BeginBold();
    m_resultText->WriteText(wxString::Format("%d. %s", m_CheckStep, text));
    m_resultText->EndBold();
    m_resultText->Newline();
}

void DialogSuggestion::AddInfo(const wxString& text)
{
    if (!m_resultText)
        return;

    m_resultText->WriteText(text);
    m_resultText->Newline();
}

void DialogSuggestion::AddWarning(const wxString& text)
{
    if (!m_resultText)
        return;

    m_resultText->BeginBold();
    m_resultText->BeginTextColour(wxColour("ORANGE"));
    m_resultText->WriteText("Warning: " + text);
    m_resultText->EndTextColour();
    m_resultText->EndBold();
    m_resultText->Newline();
}

void DialogSuggestion::AddError(const wxString& text)
{
    if (!m_resultText)
        return;

    m_resultText->BeginBold();
    m_resultText->BeginTextColour(*wxRED);
    m_resultText->WriteText("Error: " + text);
    m_resultText->EndTextColour();
    m_resultText->EndBold();
    m_resultText->Newline();
}

bool DialogSuggestion::LoadModel(const wxString& modelPath, llama_model*& currentModel)
{
    if (currentModel) {
        llama_model_free(currentModel);
        currentModel = nullptr;
    }

    wxString trimmedModelPath = TrimValue(modelPath);
    if (trimmedModelPath.IsEmpty()) {
        AddError("No model selected.");
        return false;
    }

    wxFileName modelFile(trimmedModelPath);
    if (!modelFile.FileExists()) {
        AddError("Model file does not exist: " + trimmedModelPath);
        return false;
    }

    llama_backend_init();

    llama_model_params params = llama_model_default_params();
    currentModel = llama_model_load_from_file(trimmedModelPath.mb_str().data(), params);
    if (!currentModel) {
        AddError("Failed to load model: " + trimmedModelPath);
        return false;
    }

    AddInfo("Model loaded: " + trimmedModelPath);
    return true;
}

bool DialogSuggestion::CreateSuggestion(const llama_model* currentModel)
{
    if (!currentModel) {
        AddError("No model loaded.");
        return false;
    }

    const wxString modelPath = m_modelCmb ? TrimValue(m_modelCmb->GetValue()) : wxString();
    const wxString cpuText = m_cpuCmb ? TrimValue(m_cpuCmb->GetValue()) : wxString();
    const wxString ramText = m_ramCmb ? TrimValue(m_ramCmb->GetValue()) : wxString();
    const wxString gpuText = m_gpuCmb ? TrimValue(m_gpuCmb->GetValue()) : wxString();

    const int cpuCores = std::max(1, ParseLastInteger(cpuText));
    const int ramGB = std::max(0, ParseLastInteger(ramText));
    const bool hasGpu = !gpuText.IsEmpty() && !LooksUnknownValue(gpuText);
    const bool hasGpuVram = gpuText.Contains("GB VRAM");

    const bool useChat = m_chatCheck && m_chatCheck->GetValue();
    const bool useAgent = m_agentCheck && m_agentCheck->GetValue();
    const bool useEmbedding = m_embeddingCheck && m_embeddingCheck->GetValue();
    const bool useAutocomplete = m_autocompleteCheck && m_autocompleteCheck->GetValue();

    const int nCtxTrain = std::max(0, llama_model_n_ctx_train(currentModel));
    const int nLayer = std::max(0, llama_model_n_layer(currentModel));
    const int nEmbd = std::max(0, llama_model_n_embd(currentModel));
    const uint64_t modelSize = llama_model_size(currentModel);
    const uint64_t nParams = llama_model_n_params(currentModel);
    const bool isRecurrent = llama_model_is_recurrent(currentModel);
    const bool isDiffusion = llama_model_is_diffusion(currentModel);
    const bool hasEncoder = llama_model_has_encoder(currentModel);
    const bool hasDecoder = llama_model_has_decoder(currentModel);
    const bool hasChatTemplate = llama_model_chat_template(currentModel, nullptr) != nullptr;

    int threads = std::max(1, cpuCores - 1);
    if (useAutocomplete)
        threads = std::min(threads, 4);
    else if (useEmbedding)
        threads = std::min(threads, 8);

    int ctxSize = 4096;
    if (useAutocomplete)
        ctxSize = 4096;
    else if (useEmbedding)
        ctxSize = 8192;
    else if (useAgent)
        ctxSize = 16384;
    else if (useChat)
        ctxSize = 8192;

    if (nCtxTrain > 0)
        ctxSize = std::min(ctxSize, nCtxTrain);
    if (isRecurrent)
        ctxSize = std::min(ctxSize, 4096);
    ctxSize = std::max(ctxSize, 2048);

    int batchSize = 256;
    if (ramGB >= 64)
        batchSize = 2048;
    else if (ramGB >= 32)
        batchSize = 1024;
    else if (ramGB >= 16)
        batchSize = 512;
    if (useAutocomplete)
        batchSize = std::min(batchSize, 256);

    int ubatchSize = std::min(batchSize, ramGB >= 32 ? 512 : 256);
    int parallel = useAgent ? 2 : 1;
    int gpuLayers = 0;
    if (hasGpu && nLayer > 0) {
        gpuLayers = std::max(8, nLayer / 4);
        if (ramGB >= 32)
            gpuLayers = std::max(gpuLayers, nLayer / 2);
        gpuLayers = std::min(gpuLayers, nLayer);
    }

    AddInfo("Model description: " + GetModelDescription(currentModel));
    AddInfo("Model size: " + FormatGiB(modelSize));
    AddInfo("Model parameters: " + FormatBillions(nParams));
    AddInfo(wxString::Format("Model context size (train): %d", nCtxTrain));
    AddInfo(wxString::Format("Model layers: %d", nLayer));
    AddInfo(wxString::Format("Model embedding size: %d", nEmbd));

    if (hasGpu && !hasGpuVram)
        AddWarning("GPU offload was estimated from the selected GPU name only. VRAM size is not available.");
    if (isRecurrent)
        AddWarning("Recurrent model detected. Context suggestion was kept conservative.");
    if (isDiffusion)
        AddWarning("Diffusion model detected. Generated suggestions may need manual adjustment.");
    if (useChat && !hasChatTemplate)
        AddWarning("No chat template was found in the model metadata.");
    if (useEmbedding && !hasEncoder && !hasDecoder)
        AddWarning("Embedding usage was selected, but model capabilities could not be inferred clearly.");

    wxString commonParams;
    if (!modelPath.IsEmpty())
        commonParams << " --model \"" << modelPath << "\"";
    commonParams << " --ctx-size " << ctxSize;
    commonParams << " --threads " << threads;
    commonParams << " --batch-size " << batchSize;
    if (gpuLayers > 0)
        commonParams << " --n-gpu-layers " << gpuLayers;
    if (useEmbedding)
        commonParams << " --embedding";

    if (m_cliRunParamsCheck && m_cliRunParamsCheck->GetValue()) {
        AddInfo("cli run parameters:");
        AddInfo("llama-cli" + commonParams);
    }

    if (m_serverRunParamsCheck && m_serverRunParamsCheck->GetValue()) {
        wxString serverParams = commonParams;
        serverParams << " --ubatch-size " << ubatchSize;
        serverParams << " --parallel " << parallel;
        if (useChat && hasChatTemplate)
            serverParams << " --jinja";
        AddInfo("server run parameters:");
        AddInfo("llama-server" + serverParams);
    }

    if (m_presetServerIniCheck && m_presetServerIniCheck->GetValue()) {
        AddInfo("preset server INI file:");
        if (!modelPath.IsEmpty())
            AddInfo("model=" + modelPath);
        AddInfo(wxString::Format("ctx_size=%d", ctxSize));
        AddInfo(wxString::Format("threads=%d", threads));
        AddInfo(wxString::Format("batch_size=%d", batchSize));
        AddInfo(wxString::Format("ubatch_size=%d", ubatchSize));
        AddInfo(wxString::Format("parallel=%d", parallel));
        AddInfo(wxString::Format("n_gpu_layers=%d", gpuLayers));
        AddInfo(wxString::Format("embedding=%s", useEmbedding ? "true" : "false"));
    }

    return true;
}

bool DialogSuggestion::LoadModelInfo(const ConfigFile &confFile)
{
    AddStep("Loading model information from configuration file");
    auto models = confFile.GetModels();
    return false;
}

void DialogSuggestion::OnDoItBtn(wxCommandEvent& event)
{
    m_CheckStep = 0;
    llama_model* currentModel = nullptr;

    auto cleanup = ScopeGuard {[&] { 
        if (currentModel) llama_model_free(currentModel);
        currentModel = nullptr;
        llama_backend_free();
    }};     

    if (m_resultText) {
        m_resultText->SetFont(m_resultTextFont);
        wxRichTextAttr attr;
        attr.SetFont(m_resultTextFont);
        attr.SetTextColour(*wxBLACK);
        m_resultText->SetDefaultStyle(attr);
        m_resultText->Clear();
    }

    wxString modelPath;
    wxString loadStepText = "Loading model";
    if (m_modelCmb) {
        modelPath = TrimValue(m_modelCmb->GetValue());
        wxFileName modelFile(modelPath);
        if (modelFile.FileExists())
            loadStepText += ": " + modelFile.GetFullName();
    }

    AddStep(loadStepText);

    if (!m_modelCmb) {
        AddError("Model selection is not available.");
        return;
    }

    if (!LoadModel(modelPath, currentModel))
        return;

    AddStep("Creating suggestion");
    CreateSuggestion(currentModel);
}

void DialogSuggestion::OnCloseBtn(wxCommandEvent& event)
{
    EndModal(wxID_CLOSE);
}
