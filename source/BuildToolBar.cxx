//
// BuildToolBar.cxx
//

#include "prc.hxx"
#include "BuildToolBar.hxx"

#include "../resources/fileopen.xpm"
#include "../resources/folder_open.xpm"
#include "../resources/archive-extract.xpm"

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

    wxMenu* folderMenu = new wxMenu();
    folderMenu->Append(ID_OPEN_LLAMA_SOURCE, "Open folder");
    folderMenu->Append(ID_SHOW_FILES, "Show files...");

    SetDropdownMenu(ID_OPEN_LLAMA_SOURCE, folderMenu);

    Realize();
}