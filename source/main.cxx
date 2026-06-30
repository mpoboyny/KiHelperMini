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
    
    // Disable wxWidgets GUI error dialogs and default handlers so errors
    // are reported to stderr instead of popping GUI message boxes.
    // Redirect wxLog (used by wxLogError/wxLogWarning/etc.) to stderr.
    wxLog::SetActiveTarget(new wxLogStderr());
    // Redirect other message output to stderr as well.
    wxMessageOutput::Set(new wxMessageOutputStderr());
    // Disable assert dialogs
    wxDisableAsserts();
    // Ensure fatal exception handling is disabled (no crash dialogs)
    wxHandleFatalExceptions(false);

    wxTheApp->CallOnInit();
    
    CMainDialog* dialog = new CMainDialog();
    wxTheApp->SetTopWindow(dialog);
    dialog->Show(true);
    wxTheApp->MainLoop();
    
    wxTheApp->OnExit();
    wxEntryCleanup();

    return 0;
}
