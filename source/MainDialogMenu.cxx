//
// MainDialogMenu.cxx
//

#include "prc.hxx"
#include "MainDialogMenu.hxx"

#include "../resources/edit-4.xpm"
#include "../resources/restart.xpm"
#include "../resources/build.xpm"

CMainDialogMenu::CMainDialogMenu() 
    : wxMenuBar()
{
    TrFu;
    
    wxMenu* fileMenu = new wxMenu();
    wxMenuItem* exitItem = new wxMenuItem(fileMenu, wxID_EXIT, "E&xit\tAlt-X", "Quit this program");
    exitItem->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_MENU));
    fileMenu->Append(exitItem);
    Append(fileMenu, "&File");

    fileMenu->AppendSeparator();

    // Reset menu (uses restart icon)
    wxMenuItem* resetItem = new wxMenuItem(fileMenu, ID_RESET, "&Reset and reload\tCtrl-R", "Reset config and reload application");
    resetItem->SetBitmap(wxBitmap(restart));
    fileMenu->Append(resetItem);


    // Edit Menu
    wxMenu* menuEdit = new wxMenu();
    wxMenuItem* settingsItem = new wxMenuItem(menuEdit, ID_SETTINGS, "Settings", "Open configuration settings");
    settingsItem->SetBitmap(wxBitmap(edit_xpm));
    menuEdit->Append(settingsItem);
    Append(menuEdit, "&Edit");

    wxMenu* buildMenu = new wxMenu();
    // Add llama.cpp as a direct menu item under Build
    wxMenuItem* llamaItem = new wxMenuItem(buildMenu, ID_BUILD_LLAMA, "llama.cpp", "Build llama.cpp");
    llamaItem->SetBitmap(wxBitmap(build_xpm));
    buildMenu->Append(llamaItem);
    Append(buildMenu, "&Build");

    wxMenu* helpMenu = new wxMenu();
    wxMenuItem* aboutItem = new wxMenuItem(helpMenu, wxID_ABOUT, "&About...\tF1", "Show about dialog");
    aboutItem->SetBitmap(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_MENU));
    helpMenu->Append(aboutItem);
    Append(helpMenu, "&Help");
}