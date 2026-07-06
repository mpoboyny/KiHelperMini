//
// ToolBarDropDown.cxx
//

#include "prc.hxx"
#include "ToolBarDropDown.hxx"

/*------ ToolBarDropDownItem -----*/

ToolBarDropDownItem::ToolBarDropDownItem(wxWindow* parent,
                                         const wxBitmap& bitmap,
                                         const wxString& title,
                                         const wxString& subtitle,
                                         int commandId)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
    , m_commandId(commandId)
    , m_bitmap(bitmap)
    , m_title(title)
    , m_subtitle(subtitle)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
    SetCursor(wxCursor(wxCURSOR_HAND));
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxClientDC dc(this);
    int textW = 0;
    int textH = 0;
    dc.GetTextExtent(m_title, &textW, &textH);
    int subW = 0;
    int subH = 0;
    dc.GetTextExtent(m_subtitle, &subW, &subH);

    int w = 16 + 8 + std::max(textW, subW) + 24;
    int h = std::max(16, textH + subH + 4) + 16;
    SetMinSize(wxSize(w, h));
    SetInitialSize(GetMinSize());

    Bind(wxEVT_PAINT, &ToolBarDropDownItem::OnPaint, this);
    Bind(wxEVT_LEFT_UP, &ToolBarDropDownItem::OnClick, this);
    Bind(wxEVT_ENTER_WINDOW, &ToolBarDropDownItem::OnEnterWindow, this);
    Bind(wxEVT_LEAVE_WINDOW, &ToolBarDropDownItem::OnLeaveWindow, this);
}

void ToolBarDropDownItem::PostToolbarCommand(wxWindow* source, int id)
{
    wxWindow* target = wxGetTopLevelParent(source);
    if (target == nullptr) {
        target = source;
    }

    wxCommandEvent event(wxEVT_MENU, id);
    event.SetEventObject(source);
    wxPostEvent(target, event);
}

void ToolBarDropDownItem::Highlight(bool on)
{
    if (m_hover == on) {
        return;
    }
    m_hover = on;
    Refresh();
}

void ToolBarDropDownItem::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    wxColour bg = m_hover ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)
                          : wxSystemSettings::GetColour(wxSYS_COLOUR_MENU);
    dc.SetBackground(wxBrush(bg));
    dc.Clear();

    wxColour fg = m_hover ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)
                          : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
    wxColour subFg = m_hover ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT)
                             : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

    dc.SetTextForeground(fg);
    dc.DrawBitmap(m_bitmap, 8, (GetClientSize().GetHeight() - m_bitmap.GetHeight()) / 2, true);

    wxFont titleFont = GetFont();
    dc.SetFont(titleFont);
    int textX = 8 + m_bitmap.GetWidth() + 8;
    int y = 8;
    dc.DrawText(m_title, textX, y);

    int titleW = 0;
    int titleH = 0;
    dc.GetTextExtent(m_title, &titleW, &titleH);
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT)));
    int lineY = y + titleH + 2;
    dc.DrawLine(textX, lineY, GetClientSize().GetWidth() - 8, lineY);

    wxFont subFont = GetFont();
    if (subFont.GetPointSize() > 0) {
        subFont.SetPointSize(subFont.GetPointSize() - 1);
    }
    dc.SetFont(subFont);
    dc.SetTextForeground(subFg);
    dc.DrawText(m_subtitle, textX, lineY + 3);
}

void ToolBarDropDownItem::OnEnterWindow(wxMouseEvent& event)
{
    Highlight(true);
    event.Skip();
}

void ToolBarDropDownItem::OnLeaveWindow(wxMouseEvent& event)
{
    Highlight(false);
    event.Skip();
}

void ToolBarDropDownItem::OnClick(wxMouseEvent& event)
{
    wxPopupTransientWindow* popup = wxDynamicCast(GetParent(), wxPopupTransientWindow);
    if (popup != nullptr) {
        popup->Dismiss();
    }

    PostToolbarCommand(this, m_commandId);
    event.Skip();
}

/*------ ToolBarDropDownPopup -----*/

class ToolBarDropDownPopup : public wxPopupTransientWindow
{
public:
    ToolBarDropDownPopup(wxWindow* parent, const std::vector<ToolBarDropDown::Item>& items)
        : wxPopupTransientWindow(parent, wxBORDER_SIMPLE)
    {
        SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        for (size_t i = 0; i < items.size(); ++i) {
            const ToolBarDropDown::Item& item = items[i];
            sizer->Add(new ToolBarDropDownItem(this,
                                               item.bitmap,
                                               item.title,
                                               item.subtitle,
                                               item.commandId),
                       0,
                       wxEXPAND | (i > 0 ? wxTOP : 0),
                       i > 0 ? 1 : 0);
        }

        SetSizerAndFit(sizer);
        Layout();
    }

protected:
    void OnDismiss() override
    {
        Destroy();
    }
};

ToolBarDropDown::ToolBarDropDown(wxWindow* parent)
    : m_parent(parent)
{
}

void ToolBarDropDown::AddItem(const wxBitmap& bitmap,
                              const wxString& title,
                              const wxString& subtitle,
                              int commandId)
{
    m_items.push_back({bitmap, title, subtitle, commandId});
}

void ToolBarDropDown::Popup(const wxPoint& screenPosition)
{
    if (m_parent == nullptr || m_items.empty()) {
        return;
    }

    ToolBarDropDownPopup* popup = new ToolBarDropDownPopup(m_parent, m_items);
    popup->Move(screenPosition);
    popup->Popup(m_parent);
}
