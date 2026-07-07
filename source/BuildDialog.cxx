//
// BuildDialog.cxx
//

#include "prc.hxx"
#include "BuildDialog.hxx"
#include "DownlodDialog.hxx"
#include "DialogUnzipLlama.hxx"
#include "ProcessRunner.hxx"
#include "BuildToolBar.hxx"
#include "../resources/app.xpm"

BuildDialog::BuildDialog(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "Build", wxDefaultPosition, wxSize(700, 500), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
    , m_parent(parent)
{
    TrFu;
    SetIcon(wxIcon(app_xpm));

    const wxColour contentBg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetBackgroundColour(contentBg);

    SetToolBar(new BuildToolBar(this));

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    wxFont titleFont = GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, "Build llama.cpp");
    title->SetFont(titleFont);
    top->Add(title, 0, wxALL | wxEXPAND, 12);

    wxStaticText* info = new wxStaticText(this, wxID_ANY,
        "Check the tools below.",
        wxDefaultPosition, wxSize(660, -1), wxALIGN_LEFT);
    info->Wrap(660);
    top->Add(info, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    top->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 12);

    wxStaticBoxSizer* toolsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Tools");
    toolsBox->GetStaticBox()->SetBackgroundColour(contentBg);
    wxBoxSizer* cmakeRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* cmakeLabel = new wxStaticText(this, wxID_ANY, "CMake:");
    cmakeRow->Add(cmakeLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    m_cmakePathText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_READONLY);
    m_cmakePathText->SetName("cmake_path");
    cmakeRow->Add(m_cmakePathText, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 10);
    m_checkButton = new wxButton(this, ID_CHECK_CMAKE, "Check");
    cmakeRow->Add(m_checkButton, 0, wxALL, 10);
    toolsBox->Add(cmakeRow, 0, wxEXPAND);

    wxBoxSizer* toolsRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* sourceLabel = new wxStaticText(this, wxID_ANY, "llama.cpp source:");
    toolsRow->Add(sourceLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    m_llamaSource = new wxTextCtrl(this, wxID_ANY, wxFileName(g_ConfDir + DownlodDialog::s_defSaveDir).GetFullPath(), wxDefaultPosition, wxDefaultSize);
    m_llamaSource->SetName("llama_source");
    toolsRow->Add(m_llamaSource, 1, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
    toolsBox->Add(toolsRow, 0, wxEXPAND);

    top->Add(toolsBox, 0, wxALL | wxEXPAND, 12);

    wxStdDialogButtonSizer* btns = new wxStdDialogButtonSizer();
    btns->AddButton(new wxButton(this, wxID_OK, "Run"));
    btns->AddButton(new wxButton(this, wxID_CANCEL));
    btns->Realize();
    top->Add(btns, 0, wxALIGN_CENTER | wxALL, 12);

    SetSizer(top);
    top->SetSizeHints(this);
    SetSize(700, 500);
    CentreOnParent();

    Bind(wxEVT_BUTTON, &BuildDialog::OnCheckCMake, this, ID_CHECK_CMAKE);
    Bind(wxEVT_MENU, &BuildDialog::OnOpenLlamaSource, this, ID_OPEN_LLAMA_SOURCE);
    Bind(wxEVT_MENU, &BuildDialog::OnShowFiles, this, ID_SHOW_FILES);
    Bind(wxEVT_MENU, &BuildDialog::OnDownloadLlama, this, ID_DOWNLOAD_LLAMA);
    Bind(wxEVT_MENU, &BuildDialog::OnUnzipLlama, this, ID_UNZIP_LLAMA);
    Bind(wxEVT_CLOSE_WINDOW, &BuildDialog::OnClose, this);

    m_cmakePathText->SetValue(CMakePath());
}

int BuildDialog::ShowModalLike()
{
    if (m_parent) {
        m_parent->Disable();
    }
    m_eventLoop = nullptr;
    Show();
    Raise();
    wxModalEventLoop eventLoop(this);
    m_eventLoop = &eventLoop;
    eventLoop.Run();
    m_eventLoop = nullptr;
    if (m_parent) {
        m_parent->Enable();
        m_parent->Raise();
    }
    Hide();
    return wxID_OK;
}

#ifdef _WIN32
wxString BuildDialog::CMakePath()
{
    wxString pathEnv;
    if (!wxGetEnv("PATH", &pathEnv)) {
        return wxEmptyString;
    }

    wxString exeName = "cmake.exe";
    wxStringTokenizer pathTokens(pathEnv, wxPATH_SEP, wxTOKEN_DEFAULT);
    while (pathTokens.HasMoreTokens()) {
        wxFileName candidate(pathTokens.GetNextToken(), exeName);
        if (candidate.FileExists()) {
            return candidate.GetFullPath();
        }
    }

    return wxEmptyString;
}

#else
wxString BuildDialog::CMakePath()
{
    wxString pathEnv;
    if (!wxGetEnv("PATH", &pathEnv)) {
        return wxEmptyString;
    }

    wxString exeName = "cmake";
    wxStringTokenizer pathTokens(pathEnv, wxPATH_SEP, wxTOKEN_DEFAULT);
    while (pathTokens.HasMoreTokens()) {
        wxFileName candidate(pathTokens.GetNextToken(), exeName);
        if (candidate.FileExists()) {
            return candidate.GetFullPath();
        }
    }

    return wxEmptyString;
}
#endif

void BuildDialog::OnCheckCMake(wxCommandEvent& event)
{
    wxString path = m_cmakePathText->GetValue();
    if (path.IsEmpty()) {
        ShowGenericMessageBox("Please find cmake first.", "Check cmake", wxOK | wxICON_INFORMATION, this);
        return;
    }

    ProcessRunner runner;
    wxString output = runner.Run(path, "--version");
    if (output.IsEmpty()) {
        ShowGenericMessageBox("Failed to run cmake.", "Check cmake", wxOK | wxICON_ERROR, this);
        return;
    }

    ShowGenericMessageBox(output, "CMake version", wxOK | wxICON_INFORMATION, this);
}

void BuildDialog::OnOpenLlamaSource(wxCommandEvent& event)
{
    wxString sourcePath = m_llamaSource->GetValue();
    wxDirDialog dlg(this, "Select llama.cpp source folder", wxGetCwd(),
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    wxString currPath = sourcePath.Trim();
    if (!currPath.IsEmpty())
        dlg.SetPath(currPath);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    sourcePath = dlg.GetPath();
    m_llamaSource->SetValue(sourcePath);
}

void BuildDialog::OnShowFiles(wxCommandEvent& event)
{
    TrFu;
    wxString sourcePath = m_llamaSource->GetValue().Trim();
    if (sourcePath.IsEmpty()) {
        ShowGenericMessageBox("Please select llama.cpp source folder first.", "Show files", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxFileName fn(sourcePath);
    wxString dirToOpen;
    if (fn.DirExists()) {
        dirToOpen = fn.GetFullPath();
    } else if (fn.FileExists()) {
        dirToOpen = fn.GetPath();
    } else {
        ShowGenericMessageBox("Invalid source folder.", "Show files", wxOK | wxICON_ERROR, this);
        return;
    }

#if defined(_WIN32) || defined(__gnu_linux__)
    wxString cmd;
#if defined(_WIN32)
    cmd = wxString::Format("explorer.exe \"%s\"", dirToOpen);
#else
    cmd = wxString::Format("xdg-open \"%s\"", dirToOpen);
#endif
    long pid = wxExecute(cmd, wxEXEC_ASYNC);
    if (pid != -1) {
        return;
    }

    ShowGenericMessageBox("Failed to launch file manager.", "Show files", wxOK | wxICON_ERROR, this);
#else
    ShowGenericMessageBox("Opening file manager is not supported on this platform.", "Show files", wxOK | wxICON_INFORMATION, this);
#endif
}

void BuildDialog::OnDownloadLlama(wxCommandEvent& event)
{
    DownlodDialog dlg(this);
    dlg.ShowModal();
    if (dlg.GetDownloadResult()) {
        m_llamaSource->SetValue(dlg.GetSavedFile());
    }
}

void BuildDialog::OnUnzipLlama(wxCommandEvent& event)
{
    DialogUnzipLlama dlg(this);
    dlg.ShowModal();
}

void BuildDialog::OnClose(wxCloseEvent& event)
{
    if (m_parent) {
        m_parent->Enable();
        m_parent->Raise();
    }
    Hide();

    if (m_eventLoop) {
        m_eventLoop->Exit();
    }
}
