//
// main.hxx
//

#include "prc.hxx"
#include "MainDialog.hxx"

#ifdef __WXMSW__
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
    wxDISABLE_DEBUG_SUPPORT();
    
    wxApp::SetInstance(new wxApp());

#ifdef __WXMSW__
    if (!wxEntryStart(hInstance, hPrevInstance, lpCmdLine, nCmdShow)) {
#else
    if (!wxEntryStart(argc, argv)) {
#endif
        return -1;
    }
    
    wxTheApp->CallOnInit();
    
    CMainDialog* dialog = new CMainDialog();
    wxTheApp->SetTopWindow(dialog);
    dialog->Show(true);
    wxTheApp->MainLoop();
    
    wxTheApp->OnExit();
    wxEntryCleanup();

    return 0;
}
