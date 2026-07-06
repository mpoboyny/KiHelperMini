//
// BuildToolBar.cxx
//

#include "prc.hxx"
#include "BuildToolBar.hxx"

#include <wx/popupwin.h>

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
        {
                SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
                SetCursor(wxCursor(wxCURSOR_HAND));

                wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

                wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, bitmap);
                row->Add(icon, 0, wxALL | wxALIGN_TOP, 8);

                wxBoxSizer* textSizer = new wxBoxSizer(wxVERTICAL);

                wxStaticText* titleText = new wxStaticText(this, wxID_ANY, title);
                wxFont titleFont = titleText->GetFont();
                titleFont.SetWeight(wxFONTWEIGHT_BOLD);
                titleText->SetFont(titleFont);

                wxStaticText* subtitleText = new wxStaticText(this, wxID_ANY, subtitle);
                wxFont subtitleFont = subtitleText->GetFont();
                if (subtitleFont.GetPointSize() > 0) {
                        subtitleFont.SetPointSize(subtitleFont.GetPointSize() - 1);
                        subtitleText->SetFont(subtitleFont);
                }
                subtitleText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

                textSizer->Add(titleText, 0, wxBOTTOM, 2);
                textSizer->Add(subtitleText, 0);

                row->Add(textSizer, 1, wxALL | wxALIGN_CENTER_VERTICAL, 8);
                SetSizerAndFit(row);

                BindClicks(this);
                BindClicks(icon);
                BindClicks(titleText);
                BindClicks(subtitleText);
        }

private:
        int m_commandId;

        void BindClicks(wxWindow* window)
        {
                window->Bind(wxEVT_LEFT_UP, &FolderPopupItem::OnClick, this);
                window->Bind(wxEVT_ENTER_WINDOW, &FolderPopupItem::OnEnterWindow, this);
                window->Bind(wxEVT_LEAVE_WINDOW, &FolderPopupItem::OnLeaveWindow, this);
        }

        void Highlight(bool on)
        {
                SetBackgroundColour(on ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)
                                                                : wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

                wxWindowList& children = GetChildren();
                for (wxWindowList::compatibility_iterator it = children.GetFirst(); it; it = it->GetNext()) {
                        it->GetData()->SetBackgroundColour(GetBackgroundColour());
                }

                Refresh();
                Update();
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