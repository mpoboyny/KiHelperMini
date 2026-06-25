//
// BuildDialogWin.cxx
//

#include "prc.hxx"
#include "BuildDialogWin.hxx"
#include "ProcessRunner.hxx"
#include <wx/statline.h>

enum
{
    ID_OPEN_LLAMA_SOURCE = wxID_HIGHEST + 200,
    ID_CHECK_CMAKE = wxID_HIGHEST + 201
};

BuildDialogWin::BuildDialogWin(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Build (Windows)", wxDefaultPosition, wxSize(700, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    TrFu;

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
    wxBoxSizer* cmakeRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* cmakeLabel = new wxStaticText(this, wxID_ANY, "Cmake:");
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

    m_llamaSource = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
    m_llamaSource->SetName("llama_source");
    toolsRow->Add(m_llamaSource, 1, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);

    toolsBox->Add(toolsRow, 0, wxEXPAND);

    wxBoxSizer* toolsActionRow = new wxBoxSizer(wxHORIZONTAL);
    m_sourceButton = new wxButton(this, ID_OPEN_LLAMA_SOURCE, "llama.cpp source");
    toolsActionRow->Add(m_sourceButton, 0, wxALL, 10);
    toolsActionRow->AddStretchSpacer(1);
    toolsBox->Add(toolsActionRow, 0, wxEXPAND);

    top->Add(toolsBox, 0, wxALL | wxEXPAND, 12);

    wxStaticBoxSizer* commandBox = new wxStaticBoxSizer(wxVERTICAL, this, "Build command");
    wxStaticText* commandText = new wxStaticText(this, wxID_ANY,
        "nmake /f NMakefile clean runr", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    commandText->SetFont(commandText->GetFont().Bold());
    commandBox->Add(commandText, 0, wxALL | wxEXPAND, 10);
    top->Add(commandBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 12);

    wxStdDialogButtonSizer* btns = new wxStdDialogButtonSizer();
    btns->AddButton(new wxButton(this, wxID_OK, "Run"));
    btns->AddButton(new wxButton(this, wxID_CANCEL));
    btns->Realize();
    top->Add(btns, 0, wxALIGN_CENTER | wxALL, 12);

    SetSizer(top);
    top->SetSizeHints(this);
    SetSize(700, 500);
    CentreOnParent();

    Bind(wxEVT_BUTTON, &BuildDialogWin::OnCheckCMake, this, ID_CHECK_CMAKE);
    Bind(wxEVT_BUTTON, &BuildDialogWin::OnOpenLlamaSource, this, ID_OPEN_LLAMA_SOURCE);

    m_cmakePathText->SetValue(CMakePath());
}

wxString BuildDialogWin::CMakePath()
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

void BuildDialogWin::OnCheckCMake(wxCommandEvent& event)
{
    wxString path = m_cmakePathText->GetValue();
    if (path.IsEmpty()) {
        wxMessageBox("Please find cmake first.", "Check cmake", wxOK | wxICON_INFORMATION, this);
        return;
    }

    ProcessRunner runner;
    wxString output = runner.Run(path, "--version");
    if (output.IsEmpty()) {
        wxMessageBox("Failed to run cmake.", "Check cmake", wxOK | wxICON_ERROR, this);
        return;
    }

    wxMessageBox(output, "CMake version", wxOK | wxICON_INFORMATION, this);
}

void BuildDialogWin::OnOpenLlamaSource(wxCommandEvent& event)
{
    wxString sourcePath = wxFileName::FileName(wxGetCwd()).GetVolume() + wxFileName::GetPathSeparator() + "source";
    if (!wxDirExists(sourcePath)) {
        sourcePath = wxGetCwd();
    }

    m_llamaSource->SetValue(sourcePath);
    wxLaunchDefaultApplication(sourcePath);
}
