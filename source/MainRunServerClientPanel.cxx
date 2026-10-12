//
// MainRunServerClientPanel.cxx
//

#include "prc.hxx"
#include "DialogServerStatus.hxx"
#include "MainRunServerClientPanel.hxx"

CMainRunServerClientPanel::CMainRunServerClientPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* groupBox = new wxStaticBox(this, wxID_ANY, "Start server client");
    wxStaticBoxSizer* groupSizer = new wxStaticBoxSizer(groupBox, wxVERTICAL);

    wxString clientChoices[] = { "Web Browser" };
    wxRadioBox* clientRadioBox = new wxRadioBox(groupBox, wxID_ANY, wxEmptyString,
                                                  wxDefaultPosition, wxDefaultSize,
                                                  WXSIZEOF(clientChoices), clientChoices,
                                                  1, wxRA_SPECIFY_ROWS);
    clientRadioBox->SetSelection(0);

    wxBoxSizer* clientSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* runButton = new wxButton(groupBox, wxID_ANY, "Run...", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    clientSizer->Add(clientRadioBox, 0, wxALIGN_CENTER_VERTICAL);
    clientSizer->AddStretchSpacer(1);
    clientSizer->Add(runButton, 0, wxALIGN_CENTER_VERTICAL);

    groupSizer->Add(clientSizer, 0, wxEXPAND | wxALL, 5);

    mainSizer->Add(groupSizer, 0, wxEXPAND);
    SetSizer(mainSizer);

    Bind(wxEVT_BUTTON, &CMainRunServerClientPanel::OnRun, this, runButton->GetId());
}

void CMainRunServerClientPanel::OnRun(wxCommandEvent& event)
{
    if (!DialogServerStatus::IsServerRunning())
    {
        wxMessageBox("Llama server is not running.", "Server not running", wxOK | wxICON_ERROR, this);
    }
}
