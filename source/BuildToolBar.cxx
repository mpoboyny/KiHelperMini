//
// BuildToolBar.cxx
//

#include "prc.hxx"
#include "BuildToolBar.hxx"

#include "../resources/fileopen.xpm"
#include "../resources/folder_open.xpm"
#include "../resources/archive-extract.xpm"

namespace
{
void PostToolbarCommand(wxWindow* source, int id)
{
        wxWindow* target = wxGetTopLevelParent(source);
        if (target == nullptr) {
                target = source;
        }

        wxCommandEvent event(wxEVT_MENU, id);
        event.SetEventObject(source);
        wxPostEvent(target, event);
}

class FolderPopupItem : public wxPanel
{
public:
        FolderPopupItem(wxWindow* parent,
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

                                Bind(wxEVT_PAINT, &FolderPopupItem::OnPaint, this);
                                Bind(wxEVT_LEFT_UP, &FolderPopupItem::OnClick, this);
                                Bind(wxEVT_ENTER_WINDOW, &FolderPopupItem::OnEnterWindow, this);
                                Bind(wxEVT_LEAVE_WINDOW, &FolderPopupItem::OnLeaveWindow, this);
        }

private:
        int m_commandId;
                wxBitmap m_bitmap;
                wxString m_title;
                wxString m_subtitle;
                bool m_hover = false;

        void Highlight(bool on)
        {
                                if (m_hover == on) {
                                                return;
                                }
                                m_hover = on;
                                Refresh();
        }

                void OnPaint(wxPaintEvent& event)
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

        void OnEnterWindow(wxMouseEvent& event)
        {
                Highlight(true);
                event.Skip();
        }

        void OnLeaveWindow(wxMouseEvent& event)
        {
                Highlight(false);
                event.Skip();
        }

        void OnClick(wxMouseEvent& event)
        {
                wxPopupTransientWindow* popup = wxDynamicCast(GetParent(), wxPopupTransientWindow);
                if (popup != nullptr) {
                        popup->Dismiss();
                }

                PostToolbarCommand(this, m_commandId);
                event.Skip();
        }
};

class FolderPopup : public wxPopupTransientWindow
{
public:
        explicit FolderPopup(wxWindow* parent)
                : wxPopupTransientWindow(parent, wxBORDER_SIMPLE)
        {
                SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

                wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
                sizer->Add(new FolderPopupItem(this,
                                                                           wxBitmap(folder_open_xpm),
                                                                           "Open folder",
                                                                           "Open the llama.cpp source folder",
                                                                           ID_OPEN_LLAMA_SOURCE),
                                   0, wxEXPAND);
                sizer->Add(new FolderPopupItem(this,
                                                                           wxBitmap(fileopen_xpm),
                                                                           "Show files...",
                                                                           "Open the folder in your file manager",
                                                                           ID_SHOW_FILES),
                                   0, wxEXPAND | wxTOP, 1);

                SetSizerAndFit(sizer);
                Layout();
        }

protected:
        void OnDismiss() override
        {
                Destroy();
        }
};
} // namespace

BuildToolBar::BuildToolBar(wxWindow* parent)
    : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_HORZ_LAYOUT | wxTB_NODIVIDER)
{
    TrFu;

    SetToolBitmapSize(wxSize(16, 16));

    AddTool(ID_OPEN_LLAMA_SOURCE, "llama.cpp folder...",
            wxBitmapBundle::FromBitmap(wxBitmap(folder_open_xpm)),
            "Open llama.cpp source folder", wxITEM_DROPDOWN);

    AddTool(ID_DOWNLOAD_LLAMA, "Download llama.cpp...",
            wxBitmapBundle::FromBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR, wxSize(16, 16))),
            "Download llama.cpp master", wxITEM_NORMAL);

    AddTool(ID_UNZIP_LLAMA, "Unzip llama.cpp source...",
            wxBitmapBundle::FromBitmap(wxBitmap(archive_extract)),
            "Unzip llama.cpp source", wxITEM_NORMAL);

        Bind(wxEVT_TOOL_DROPDOWN, [this](wxCommandEvent& event) {
                if (event.GetId() != ID_OPEN_LLAMA_SOURCE) {
                        event.Skip();
                        return;
                }

                FolderPopup* popup = new FolderPopup(this);
                wxPoint popupPos = ClientToScreen(wxPoint(0, GetSize().GetHeight()));
                popup->Move(popupPos);
                popup->Popup(this);
        }, ID_OPEN_LLAMA_SOURCE);

    Realize();
}