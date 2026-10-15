//
// MainRunServerClientPanel.cxx
//

#include "prc.hxx"
#include "ConfigFile.hxx"
#include "DialogRunOpencode.hxx"
#include "DialogServerStatus.hxx"
#include "MainRunServerClientPanel.hxx"
#include "MainRunServerPanel.hxx"
#include "ProcessRunner.hxx"

CMainRunServerClientPanel::CMainRunServerClientPanel(wxWindow* parent, const CMainRunServerPanel* serverPanel)
    : wxPanel(parent, wxID_ANY)
    , m_serverPanel(serverPanel)
    , m_webRadioButton(nullptr)
    , m_openCodeRadioButton(nullptr)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* groupBox = new wxStaticBox(this, wxID_ANY, "Start server client");
    wxStaticBoxSizer* groupSizer = new wxStaticBoxSizer(groupBox, wxVERTICAL);

    m_webRadioButton = new wxRadioButton(groupBox, wxID_ANY, "Web Browser", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_webRadioButton->SetValue(true);
    m_openCodeRadioButton = new wxRadioButton(groupBox, wxID_ANY, "opencode");

    wxBoxSizer* clientSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* runButton = new wxButton(groupBox, wxID_ANY, "Run...", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    clientSizer->Add(m_webRadioButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    clientSizer->Add(m_openCodeRadioButton, 0, wxALIGN_CENTER_VERTICAL);
    clientSizer->AddStretchSpacer(1);
    clientSizer->Add(runButton, 0, wxALIGN_CENTER_VERTICAL);

    groupSizer->Add(clientSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    mainSizer->Add(groupSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    SetSizer(mainSizer);

    Bind(wxEVT_BUTTON, &CMainRunServerClientPanel::OnRun, this, runButton->GetId());
}

void CMainRunServerClientPanel::OnRun(wxCommandEvent& event)
{
    if (m_webRadioButton->GetValue()) {
        RunWeb();
    }
    else if (m_openCodeRadioButton->GetValue()) {
        RunOpenCode();
    }
}

void CMainRunServerClientPanel::RunWeb()
{
    if (DialogServerStatus::IsServerRunning()) {
        wxString serverUrl;
        if (!m_serverPanel || !m_serverPanel->GetServerUrl(serverUrl)) {
            wxMessageBox("Server parameters must contain a valid --host and --port.", "Server address error", wxOK | wxICON_ERROR, this);
            return;
        }
        ProcessRunner runner;
        if (!runner.RunDefaultBrowserAsync(serverUrl)) {
            wxMessageBox("Could not start the default web browser.", "Browser error", wxOK | wxICON_ERROR, this);
        }
    }
    else {
        wxMessageBox("Llama server is not running.", "Server not running", wxOK | wxICON_ERROR, this);
    }
}

void CMainRunServerClientPanel::RunOpenCode()
{
    if (DialogServerStatus::IsServerRunning()) {
        DialogRunOpencode dlg(this, m_serverPanel->GetConfigFile());
        dlg.ShowModal();
    }
    else {
        wxMessageBox("Llama server is not running.", "Server not running", wxOK | wxICON_ERROR, this);
    }
}
