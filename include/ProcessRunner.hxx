//
// ProcessRunner.hxx
//

#ifndef PROCESSRUNNER_HXX
#define PROCESSRUNNER_HXX

#include "prc.hxx"

class ProcessRunner
{
    static const int s_DefaultWidth = 800;
    static const int s_DefaultHeight = 600;

public:
    ProcessRunner() = default;
    ~ProcessRunner() = default;

    wxString Run(const wxString& exePath, const wxString& args);
    bool RunAsyncInNewWindow(const wxString& scriptPath, int displayIndex, const wxString title);
    bool RunServerAsyncInNewWindow(const wxString& scriptContent, int displayIndex, const wxString title, bool hidden = false);
    bool RunSyncInNewWindow(const wxString& scriptPath, int displayIndex, const wxString title);
};

#endif // PROCESSRUNNER_HXX
