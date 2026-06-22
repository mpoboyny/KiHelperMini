//
// MainDialogMenu.cxx
//

#include "prc.hxx"
#include "MainDialogMenu.hxx"

#include "../resources/edit-4.xpm"

CMainDialogMenu::CMainDialogMenu() 
    : wxMenuBar()
{
    TrFu;
    
    wxMenu* fileMenu = new wxMenu();
    wxMenuItem* exitItem = new wxMenuItem(fileMenu, wxID_EXIT, "E&xit\tAlt-X", "Quit this program");
    exitItem->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_MENU));
    fileMenu->Append(exitItem);
    Append(fileMenu, "&File");

    // Edit Menu
    wxMenu *menuEdit = new wxMenu;
    wxMenuItem* settingsItem = new wxMenuItem(menuEdit, ID_SETTINGS, "Settings", "Open configuration settings");
    settingsItem->SetBitmap(wxBitmap(edit_xpm));
    menuEdit->Append(settingsItem);
    Append(menuEdit, "&Edit");

    wxMenu* helpMenu = new wxMenu();
    wxMenuItem* aboutItem = new wxMenuItem(helpMenu, wxID_ABOUT, "&About...\tF1", "Show about dialog");
    aboutItem->SetBitmap(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_MENU));
    helpMenu->Append(aboutItem);
    Append(helpMenu, "&Help");
}