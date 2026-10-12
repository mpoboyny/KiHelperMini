//
// MainRunServerClientPanel.cxx
//

#include "prc.hxx"
#include "DialogServerStatus.hxx"
#include "MainRunServerClientPanel.hxx"
#include "MainRunServerPanel.hxx"
#include "ProcessRunner.hxx"

CMainRunServerClientPanel::CMainRunServerClientPanel(wxWindow* parent, const CMainRunServerPanel* serverPanel)
    : wxPanel(parent, wxID_ANY)
    , m_serverPanel(serverPanel)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* groupBox = new wxStaticBox(this, wxID_ANY, "Start server client");
    wxStaticBoxSizer* groupSizer = new wxStaticBoxSizer(groupBox, wxVERTICAL);

    wxRadioButton* clientRadioButton = new wxRadioButton(groupBox, wxID_ANY, "Web Browser");
    clientRadioButton->SetValue(true);

    wxBoxSizer* clientSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* runButton = new wxButton(groupBox, wxID_ANY, "Run...", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    clientSizer->Add(clientRadioButton, 0, wxALIGN_CENTER_VERTICAL);
    clientSizer->AddStretchSpacer(1);
    clientSizer->Add(runButton, 0, wxALIGN_CENTER_VERTICAL);

    groupSizer->Add(clientSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    mainSizer->Add(groupSizer, 0, wxEXPAND);
    SetSizer(mainSizer);

    Bind(wxEVT_BUTTON, &CMainRunServerClientPanel::OnRun, this, runButton->GetId());
}

void CMainRunServerClientPanel::OnRun(wxCommandEvent& event)
{
    if (DialogServerStatus::IsServerRunning())
    {
        wxString serverUrl;
        if (!m_serverPanel || !m_serverPanel->GetServerUrl(serverUrl))
        {
            wxMessageBox("Server parameters must contain a valid --host and --port.", "Server address error", wxOK | wxICON_ERROR, this);
            return;
        }

        ProcessRunner runner;
        if (!runner.RunDefaultBrowserAsync(serverUrl))
        {
            wxMessageBox("Could not start the default web browser.", "Browser error", wxOK | wxICON_ERROR, this);
        }
    }
    else
    {
        wxMessageBox("Llama server is not running.", "Server not running", wxOK | wxICON_ERROR, this);
    }
}
