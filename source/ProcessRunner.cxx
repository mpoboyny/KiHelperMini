//
// ProcessRunner.cxx
//

#include "prc.hxx"
#include "ProcessRunner.hxx"

wxString ProcessRunner::Run(const wxString& exePath, const wxString& args)
{
    wxString output;
    wxArrayString stdoutLines;
    wxArrayString stderrLines;
    wxString command = wxString::Format("\"%s\" %s", exePath, args);

    long exitCode = wxExecute(command, stdoutLines, stderrLines, wxEXEC_SYNC);
    if (exitCode == -1) {
        return wxEmptyString;
    }

    auto appendLines = [&](const wxArrayString& lines) {
        for (const auto& line : lines) {
            if (!output.IsEmpty()) {
                output += "\n";
            }
            output += line;
        }
    };

    appendLines(stdoutLines);
    if (output.IsEmpty()) {
        appendLines(stderrLines);
    }

    return output;
}

#ifdef _WIN32

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath)
{
    TrFu;
    // Launch script in a new console window using cmd's start command.
    wxString command = wxString::Format("cmd /c start \"\" \"%s\"", scriptPath);
    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

#elif defined(__gnu_linux__)

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath)
{
    TrFu;
    wxString command = wxString::Format("xterm -hold -e \"%s\"", scriptPath);
    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

#else

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath)
{
    TrFu;
    return false;
}

#endif

