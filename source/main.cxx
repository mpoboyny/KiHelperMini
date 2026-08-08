//
// main.hxx
//

#include "prc.hxx"
#include "MainDialog.hxx"
#include "WaitDialog.hxx"

#ifdef __WXMSW__
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
    TrFu
    WaitDialog::Show("Initializing application...");

    wxDISABLE_DEBUG_SUPPORT();
    SetWorkingDir();
    wxApp::SetInstance(new wxApp());

#ifdef __WXMSW__
    if (!wxEntryStart(hInstance, hPrevInstance, lpCmdLine, nCmdShow)) {
#else
    if (!wxEntryStart(argc, argv)) {
#endif
        return -1;
    }
    TrP;
    // Disable wxWidgets GUI error dialogs and default handlers so errors
    // are reported via our tracing system (Tr/TrStr) instead of stderr
    // or modal message boxes.
    // Small wxLog target that forwards messages to TrStr
    class WxLogToTrace : public wxLog {
    protected:
        void DoLogTextAtLevel(wxLogLevel WXUNUSED(level), const wxString& msg) override
        {
            TrFu;
            TrStr(msg);
        }
    };
    TrP;
    // Message output that forwards to TrStr
    class WxMessageOutputToTrace : public wxMessageOutput {
    public:
        void Output(const wxString& str) override
        {
            TrFu;
            TrStr(str);
        }
    };

    // Install our trace-based targets
    wxLog::SetActiveTarget(new WxLogToTrace());
    wxMessageOutput::Set(new WxMessageOutputToTrace());

    // Disable assert dialogs and fatal-exception dialogs
    wxDisableAsserts();
    wxHandleFatalExceptions(false);

    wxTheApp->CallOnInit();
    TrP;
    CMainDialog* dialog = new CMainDialog();
    wxTheApp->SetTopWindow(dialog);
    dialog->Show(true);

    WaitDialog::Hide();

    wxTheApp->MainLoop();
    
    wxTheApp->OnExit();
    wxEntryCleanup();

    return 0;
}
