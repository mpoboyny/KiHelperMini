//
// MainRunToolBar.cxx
//

#include "prc.hxx"
#include "MainRunToolBar.hxx"

MainRunToolBar::MainRunToolBar(wxWindow* parent)
    : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_HORZ_LAYOUT | wxTB_NODIVIDER)
{
    TrFu;

    SetToolBitmapSize(wxSize(16, 16));

    AddTool(ID_RUN_SAMPLE,
            "Run sample",
            wxBitmapBundle::FromBitmap(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_TOOLBAR, wxSize(16, 16))),
            "Example execute action",
            wxITEM_NORMAL);

    Realize();
}