//
// MainRunPanel.cxx
//

#include "prc.hxx"
#include "MainRunPanel.hxx"
#include "MainRunToolBar.hxx"

CMainRunPanel::CMainRunPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_toolBar(nullptr)
{
    TrFu;

    wxStaticBox* groupBox = new wxStaticBox(this, wxID_ANY, "Execute");
    wxStaticBoxSizer* runSizer = new wxStaticBoxSizer(groupBox, wxVERTICAL);

    m_toolBar = new MainRunToolBar(groupBox);
    runSizer->Add(m_toolBar, 0, wxEXPAND | wxALL, 4);

    wxStaticText* infoText = new wxStaticText(this, wxID_ANY, "Execution controls will be here.");
    runSizer->Add(infoText, 0, wxALL, 4);

    SetSizer(runSizer);
    Bind(wxEVT_TOOL, &CMainRunPanel::OnRunSample, this, ID_RUN_SAMPLE);

    Layout();
}

void CMainRunPanel::OnRunSample(wxCommandEvent& WXUNUSED(event))
{
    ShowGenericMessageBox("Sample execute action.", "Execute", wxOK | wxICON_INFORMATION, this);
}