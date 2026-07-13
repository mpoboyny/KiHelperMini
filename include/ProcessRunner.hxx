//
// ProcessRunner.hxx
//

#ifndef PROCESSRUNNER_HXX
#define PROCESSRUNNER_HXX

#include "prc.hxx"

class ProcessRunner
{
public:
    ProcessRunner() = default;
    ~ProcessRunner() = default;

    wxString Run(const wxString& exePath, const wxString& args);
    bool RunAsyncInNewWindow(const wxString& scriptPath, int displayIndex);
};

#endif // PROCESSRUNNER_HXX
