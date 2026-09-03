//
// MainRunServerPanel.cxx
//

#include "prc.hxx"
#include "ConfigFile.hxx"
#include "DialogServerStatus.hxx"
#include "MainRunServerPanel.hxx"
#include "ProcessRunner.hxx"
#include "ScriptHandler.hxx"

/*static*/
const wxString CMainRunServerPanel::s_ServerFileName = "llama-server";

namespace
{
    wxString ExpandUserHomePath(const wxString& path)
    {
        if (path == "~") {
            return wxGetHomeDir();
        }

        if (path.StartsWith("~/") || path.StartsWith("~\\")) {
            wxString suffix = path.Mid(2);
            while (!suffix.IsEmpty() && (suffix[0] == '/' || suffix[0] == '\\')) {
                suffix.Remove(0, 1);
            }

            wxString sep = wxString::Format("%c", wxFILE_SEP_PATH);
            suffix.Replace("\\", sep);
            suffix.Replace("/", sep);

            wxString home = wxGetHomeDir();
            if (home.EndsWith("/") || home.EndsWith("\\")) {
                return home + suffix;
            }
            return home + sep + suffix;
        }

        return path;
    }

    bool IsDirectoryPath(const wxString& path)
    {
        if (path.IsEmpty()) {
            return false;
        }

        if (wxDirExists(path)) {
            return true;
        }

        wxChar lastChar = path.Last();
        return lastChar == '/' || lastChar == '\\';
    }

    wxString BuildServerPathFromBase(const wxString& path)
    {
        if (path.IsEmpty()) {
            return path;
        }

        if (IsDirectoryPath(path)) {
            wxFileName fn;
            fn.AssignDir(path);
            fn.SetFullName("llama-server");
            return fn.GetFullPath();
        }

        wxFileName fn(path);
        if (fn.GetFullName().CmpNoCase("llama-cli") == 0) {
            fn.SetFullName("llama-server");
        }
        return fn.GetFullPath();
    }

    bool IsChatOnlyParameter(const ConfigFile::ChatParameter& param)
    {
        return param.Name == "-cnv" || param.Name == "--jinja";
    }

    wxString GetDefaultServerPath(const ConfigFile& confFile)
    {
        wxString res = BuildServerPathFromBase(confFile.GetServerDefPath());
        if (!res.IsEmpty()) {
            return res;
        }

        res = BuildServerPathFromBase(confFile.GetChatDefPath());
        if (res.IsEmpty()) {
            res = BuildServerPathFromBase(confFile.GetLLamaBinPath());
        }

        return res;
    }
}

CMainRunServerPanel::CMainRunServerPanel(wxWindow* parent, const ConfigFile &confFile)
    : wxPanel(parent, wxID_ANY)
    , m_parentWindow(parent)
{
    TrFu;

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* runGroupBox = new wxStaticBox(this, wxID_ANY, "Run server");
    wxStaticBoxSizer* runSizer = new wxStaticBoxSizer(runGroupBox, wxVERTICAL);

    wxBoxSizer* fileRowSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* labelFilePath = new wxStaticText(runGroupBox, wxID_ANY, "File path:");

    m_textServerFilePath = new wxTextCtrl(runGroupBox, wxID_ANY, GetDefaultServerPath(confFile), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    wxButton* btnSelectFile = new wxButton(runGroupBox, ID_SELECT_SERVER_FILE, "Select file");

    fileRowSizer->Add(labelFilePath, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    fileRowSizer->Add(m_textServerFilePath, 1, wxEXPAND | wxRIGHT, 5);
    fileRowSizer->Add(btnSelectFile, 0, wxALIGN_CENTER_VERTICAL);

    wxBoxSizer* paramRowSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* labelParams = new wxStaticText(runGroupBox, wxID_ANY, "Parameters:");
    paramRowSizer->Add(labelParams, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_textServerParams = new wxTextCtrl(runGroupBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
    paramRowSizer->Add(m_textServerParams, 1, wxEXPAND | wxRIGHT, 5);

    ConfigFile::ChatParameterList params = confFile.GetServerParameters();
    if (params.empty()) {
        params = confFile.GetChatParameters();
    }

    for (const auto& param : params) {
        if (IsChatOnlyParameter(param)) {
            continue;
        }
        wxString paramText = param.Name;
        if (!param.Value.IsEmpty()) {
            paramText += " " + param.Value;
        }
        m_textServerParams->AppendText(paramText + " ");
    }

    auto models = confFile.GetModels();
    if (models.empty()) {
        m_Modell = "No models configured";
    }
    for (const auto& model : models) {
        if (model.Current) {
            m_Modell = model.Path;
            break;
        }
    }

    wxBoxSizer* doSizer = new wxBoxSizer(wxHORIZONTAL);
    m_buttServerDoIt = new wxButton(runGroupBox, ID_RUN_SERVER, "Do it", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    
    m_checkBox = new wxCheckBox(runGroupBox, ID_RUN_SERVER_DEAMON, "Run in background", wxDefaultPosition);
    m_checkBox->SetValue(false);
    
    m_buttServerShowHelp = new wxButton(runGroupBox, ID_RUN_SERVER_HELP, "Help", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    m_buttServerStatus = new wxButton(runGroupBox, ID_SHOW_SERVER_STATUS, "Show server status", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    m_buttServerCopyScript = new wxButton(runGroupBox, ID_COPY_SERVER_SCRIPT, "Copy script to clipboard", wxDefaultPosition, wxSize(-1, FromDIP(28)));

    doSizer->Add(m_buttServerDoIt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    doSizer->Add(m_checkBox, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    doSizer->Add(m_buttServerShowHelp, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    doSizer->Add(m_buttServerStatus, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    doSizer->Add(m_buttServerCopyScript, 0, wxALIGN_CENTER_VERTICAL);

    runSizer->Add(fileRowSizer, 0, wxEXPAND | wxALL, 10);
    runSizer->Add(paramRowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    runSizer->Add(doSizer, 0, wxEXPAND | wxLEFT | wxBOTTOM, 10);

    mainSizer->Add(runSizer, 1, wxEXPAND | wxALL, 10);
    SetSizer(mainSizer);

    Bind(wxEVT_BUTTON, &CMainRunServerPanel::OnSelectFile, this, ID_SELECT_SERVER_FILE);
    Bind(wxEVT_BUTTON, &CMainRunServerPanel::OnRunServer, this, ID_RUN_SERVER);
    Bind(wxEVT_BUTTON, &CMainRunServerPanel::OnRunServerHelp, this, ID_RUN_SERVER_HELP);
    Bind(wxEVT_BUTTON, &CMainRunServerPanel::OnShowServerStatus, this, ID_SHOW_SERVER_STATUS);
    Bind(wxEVT_BUTTON, &CMainRunServerPanel::OnCopyServerScript, this, ID_COPY_SERVER_SCRIPT);

    Layout();
}

void CMainRunServerPanel::OnSelectFile(wxCommandEvent& event)
{
    TrFu;
    wxString currentPath = m_textServerFilePath->GetValue();
    wxString defaultDir = "";
    wxString defaultFile = "";

    if (!currentPath.IsEmpty())
    {
        wxFileName fn(currentPath);
        defaultDir = fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
        defaultFile = fn.GetFullName();
    }

    TrStr(defaultDir);
    TrStr(defaultFile);

    wxFileDialog openFileDialog(
        this,
        "Select llama-server binary",
        defaultDir,
        defaultFile,
        "All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (!defaultDir.IsEmpty())
    {
        openFileDialog.SetDirectory(defaultDir);
    }
    if (!defaultFile.IsEmpty())
    {
        openFileDialog.SetFilename(defaultFile);
    }

    if (openFileDialog.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    if (m_textServerFilePath)
    {
        m_textServerFilePath->SetValue(openFileDialog.GetPath());
    }
}

void CMainRunServerPanel::OnRunServer(wxCommandEvent &event)
{
    TrFu;
    // TrStr(m_Modell);
    wxString llamaBinPath = ExpandUserHomePath(m_textServerFilePath->GetValue());
    wxString serverParams = wxString::Format("-m %s %s", m_Modell, m_textServerParams->GetValue());
    wxString scriptContent;
    if (m_checkBox->GetValue()) {
        TrStr(g_SrvLogFile);
        ScriptReplacementsList replacements = {
            { "<!-- llama_bin -->", llamaBinPath },
            { "<!-- llama_param -->", serverParams },
            { "<!-- llama_pipe -->", "" },
            { "<!-- llama_srv_log -->", g_SrvLogFile }
        };
        scriptContent = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunLlamaPathServ, replacements);
    }
    else {
        ScriptReplacementsList replacements = {
            { "<!-- llama_bin -->", llamaBinPath },
            { "<!-- llama_param -->", serverParams },
            { "<!-- llama_pipe -->", "" },
            { "<!-- llama_srv_log -->", "" }
        };
        scriptContent = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunLlamaPathServ, replacements);
    }
    // TrStr(scriptContent);
    ProcessRunner runner;
    runner.RunServerAsyncInNewWindow(scriptContent, wxDisplay::GetFromWindow(this), "Llama server", m_checkBox->GetValue());
}

void CMainRunServerPanel::OnCopyServerScript(wxCommandEvent &event)
{
    TrFu;
    wxString llamaBinPath = ExpandUserHomePath(m_textServerFilePath->GetValue());
    wxString serverParams = wxString::Format("-m %s %s", m_Modell, m_textServerParams->GetValue());
    wxString scriptContent;
    if (m_checkBox->GetValue()) {
        TrStr(g_SrvLogFile);
        ScriptReplacementsList replacements = {
            { "<!-- llama_bin -->", llamaBinPath },
            { "<!-- llama_param -->", serverParams },
            { "<!-- llama_pipe -->", "" },
            { "<!-- llama_srv_log -->", g_SrvLogFile }
        };
        scriptContent = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunLlamaPathServ, replacements);
    }
    else {
        ScriptReplacementsList replacements = {
            { "<!-- llama_bin -->", llamaBinPath },
            { "<!-- llama_param -->", serverParams },
            { "<!-- llama_pipe -->", "" },
            { "<!-- llama_srv_log -->", "" }
        };
        scriptContent = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunLlamaPathServ, replacements);
    }

    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(scriptContent));
        wxTheClipboard->Close();
    }
}

void CMainRunServerPanel::OnShowServerStatus(wxCommandEvent &event)
{
    TrFu;
    DialogServerStatus dlg(this);
    dlg.ShowModal();
}

void CMainRunServerPanel::OnRunServerHelp(wxCommandEvent &event)
{
    TrFu;
    wxString llamaBinPath = ExpandUserHomePath(m_textServerFilePath->GetValue());
    ScriptReplacementsList replacements = {
        { "<!-- llama_bin -->", llamaBinPath },
        { "<!-- llama_param -->", "--help" },
        { "<!-- llama_pipe -->", "xxx" },
        { "<!-- llama_srv_log -->", "" }
    };
    // TrStr(g_ScriptRunLlamaPath);
    wxString scriptContent = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunLlamaPath, replacements);
    // TrStr(scriptContent);
    ProcessRunner runner;
    runner.RunAsyncInNewWindow(scriptContent, wxDisplay::GetFromWindow(this), "Llama server help");
}
