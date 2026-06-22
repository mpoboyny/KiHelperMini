//
// MainDialogPanel.cxx
//

#include "prc.hxx"
#include "MainDialogPanel.hxx"

CMainDialogPanel::CMainDialogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    TrFu;

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticText* label = new wxStaticText(this, wxID_ANY, g_APP_NAME);
    mainSizer->Add(label, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 20);
    
    SetSizer(mainSizer);
    Layout();
}