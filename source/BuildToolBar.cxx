//
// BuildToolBar.cxx
//

#include "prc.hxx"
#include "BuildToolBar.hxx"
#include "ToolBarDropDown.hxx"

#include "../resources/fileopen.xpm"
#include "../resources/folder_open.xpm"
#include "../resources/archive-extract.xpm"

BuildToolBar::BuildToolBar(wxWindow* parent)
    : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_HORZ_LAYOUT | wxTB_NODIVIDER)
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
        ToolBarDropDown popup(this);
        popup.AddItem(wxBitmap(folder_open_xpm),
                      "Open folder",
                      "Open the llama.cpp source folder",
                      [this]() {
                          RunHandler(ID_OPEN_LLAMA_SOURCE);
                      });
        popup.AddItem(wxBitmap(fileopen_xpm),
                      "Show files...",
                      "Open the folder in your file manager",
                      [this]() {
                          RunHandler(ID_SHOW_FILES);
                      });
        popup.Popup(ClientToScreen(wxPoint(0, GetSize().GetHeight())));
    }, ID_OPEN_LLAMA_SOURCE);

    Realize();
}

bool BuildToolBar::RunHandler(int toolId)
{
    auto handler = m_handlers.find(toolId);
    if (handler == m_handlers.end()) {
        return false;
    }

    handler->second();
    return true;
}

void BuildToolBar::OnTool(wxCommandEvent& event)
{
    if (!RunHandler(event.GetId())) {
        event.Skip();
    }
}