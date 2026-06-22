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
}

void CMainDialogStatusBar::ShowReady()
{
    ShowMessage(GuiStatus::GuiStatus_Info, "");
}

const std::map<GuiStatus, CMainDialogStatusBar::StatusFontInfo>& CMainDialogStatusBar::Fonts()
{
    static std::map<GuiStatus, CMainDialogStatusBar::StatusFontInfo> s;
    if (s.empty()) {
        wxFont baseFont = wxFont(wxFontInfo(g_DefaultTxtSize).FaceName(g_DefaultTxtFontName));
        wxFont boldFont = wxFont(wxFontInfo(g_DefaultTxtSize).FaceName(g_DefaultTxtFontName).Bold());

        s[GuiStatus_Info] = CMainDialogStatusBar::StatusFontInfo{ baseFont, wxColour(0,0,0), wxString("Ready") };
        s[GuiStatus_Warn] = CMainDialogStatusBar::StatusFontInfo{ boldFont, wxColour(255,165,0), wxString("Warning: ") };
        s[GuiStatus_Erro] = CMainDialogStatusBar::StatusFontInfo{ boldFont, wxColour(255,0,0), wxString("Error: ") };
    }
    return s;
}

void CMainDialogStatusBar::ShowMessage(GuiStatus status, const wxString& msg)
{
    const auto &fonts = Fonts();
    auto it = fonts.find(status);
    if (it != fonts.end()) {
        SetFont(it->second.font);
        SetForegroundColour(it->second.colour);
        if (status == GuiStatus::GuiStatus_Info && msg.IsEmpty()) {
            SetStatusText(it->second.prefix, 0);
        }
        else {
            SetStatusText(it->second.prefix + msg, 0);
        }
    } else {
        SetStatusText(msg, 0);
    }
    Refresh();
}
