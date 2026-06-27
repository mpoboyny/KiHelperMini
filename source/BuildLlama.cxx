//
// BuildLlama.cxx
//

#include "prc.hxx"
#include "BuildLlama.hxx"

BuildLlama::BuildLlama(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Build llama.cpp", wxDefaultPosition, wxSize(420, 200))
{
    TrFu;

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    wxStaticText* info = new wxStaticText(this, wxID_ANY,
        "This dialog is for building llama.cpp.\n\nYour OS is not supported yet.",
        wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    top->Add(info, 1, wxALL | wxEXPAND, 12);

    wxStdDialogButtonSizer* btns = new wxStdDialogButtonSizer();
    btns->AddButton(new wxButton(this, wxID_OK));
    btns->Realize();
    top->Add(btns, 0, wxALIGN_CENTER | wxBOTTOM, 8);

    SetSizerAndFit(top);
    CentreOnParent();
}
