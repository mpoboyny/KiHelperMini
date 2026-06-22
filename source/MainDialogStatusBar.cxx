//
// MainDialogStatusBar.cxx
//

#include "MainDialogStatusBar.hxx"

CMainDialogStatusBar::CMainDialogStatusBar(wxWindow* parent)
    : wxStatusBar(parent, wxID_ANY, wxSTB_DEFAULT_STYLE | wxBORDER_SUNKEN)
{
    int widths[2];
    widths[0] = -1;
    widths[1] = 200;
    SetFieldsCount(2);
    SetStatusWidths(2, widths);
    SetStatusText("Ready", 0);
}

void CMainDialogStatusBar::ShowMessage(const wxString& msg, unsigned int field)
{
    if (field >= static_cast<unsigned int>(GetFieldsCount()))
        field = 0;
    SetStatusText(msg, field);
}
