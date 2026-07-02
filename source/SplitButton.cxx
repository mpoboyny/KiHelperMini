//
// SplitButton.cxx
//

#include "prc.hxx"
#include "SplitButton.hxx"
#include "../resources/arrow-down-2.xpm"
#include "../resources/folder_open.xpm"

SplitButton::SplitButton(wxWindow* parent, wxWindowID idMain, const wxString& labelMain, wxWindowID idArrow)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_SIMPLE)
{
    wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
    m_main = new wxButton(this, idMain, labelMain);
    wxBitmap mainBmp(folder_open_xpm);
    if (mainBmp.IsOk()) {
        m_main->SetBitmap(mainBmp);
    }

    // separator (fixed 1px width)
    wxStaticLine* sep = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, -1), wxLI_VERTICAL);

    // Use the bundled XPM arrow directly
    wxBitmap arrowBmp(arrow_down_2);

    m_arrow = new wxBitmapButton(this, idArrow, arrowBmp, wxDefaultPosition, wxSize(s_defButtHeight, s_defButtHeight), wxBORDER_NONE);
    m_arrow->SetToolTip("Options");
    
    m_arrow->SetSize(s_defButtHeight, s_defButtHeight);

    s->Add(m_main, 1, wxEXPAND, 0);
    s->Add(sep, 0, wxEXPAND | wxLEFT | wxRIGHT, 0);
    s->Add(m_arrow, 0, wxEXPAND, 0);
    SetSizerAndFit(s);
}

wxButton* SplitButton::GetMainButton() { return m_main; }
wxBitmapButton* SplitButton::GetArrowButton() { return m_arrow; }
