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

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath, int WXUNUSED(displayIndex), const wxString WXUNUSED(title))
{
    TrFu;
    // Launch script in a new console window using cmd's start command.
    wxString command = wxString::Format("cmd /c start \"\" \"%s\"", scriptPath);
    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunServerAsyncInNewWindow(const wxString& scriptContent, int displayIndex, const wxString title, bool hidden)
{
    TrFu;
    wxFile tempFile;
    wxString tempPath = wxFileName::CreateTempFileName("llama_srv_", &tempFile);
    if (!tempFile.IsOpened()) {
        return false;
    }
    tempFile.Write(scriptContent);
    tempFile.Close();

    wxFileName scriptFn(tempPath);
    scriptFn.SetExt("bat");
    wxString scriptPath = scriptFn.GetFullPath();
    if (scriptPath != tempPath && !wxRenameFile(tempPath, scriptPath, true)) {
        wxRemoveFile(tempPath);
        return false;
    }

    if (!hidden) {
        return RunAsyncInNewWindow(scriptPath, displayIndex, title);
    }

    wxString command = wxString::Format("cmd /c \"%s\"", scriptPath);
    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& scriptPath, int WXUNUSED(displayIndex), const wxString WXUNUSED(title))
{
    TrFu;
    wxString command = wxString::Format("cmd /c start /wait \"\" \"%s\"", scriptPath);
    long exitCode = wxExecute(command, wxEXEC_SYNC);
    return exitCode != -1;
}

#elif defined(__gnu_linux__)

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath, int displayIndex, const wxString title)
{
    TrFu;
    
    // Define initial grid boundaries in columns and rows
    int targetCols = 125; 
    int targetRows = 40;  
    
    // Request screen geometries safely from wxWidgets without using hardcoded pixel offsets
    wxRect screenRect = wxDisplay(displayIndex).GetClientArea();
    int posX = screenRect.x + 50; // Dynamic window manager safe padding fallback
    int posY = screenRect.y + 50;
    wxString  XtermTitle= wxString::Format("%s (Use default xterm behavior or close this window manually)", title);
    auto buildCommand = [&](bool holdWindow) {
        return wxString::Format(
            "xterm -bg black -fg white -geometry %dx%d+%d+%d "
            "-T \"%s\" " 
            // Enable dynamicLayout to automatically recalculate and reflow lines when resized
            "-xrm \"XTerm*selectToClipboard: true\" " 
            "-xrm \"XTerm*vt100.dynamicLayout: true\" " 
            "-fa Monospace -fs 12 %s-e \"%s\"",
            targetCols, targetRows, 
            posX, posY,
            XtermTitle,       
            holdWindow ? "-hold " : "",
            scriptPath
        );
    };
    
    long pid = wxExecute(buildCommand(true), wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunServerAsyncInNewWindow(const wxString& scriptContent, int displayIndex, const wxString title, bool hidden)
{
    TrFu;
    wxFile tempFile;
    wxString tempPath = wxFileName::CreateTempFileName("llama_srv_", &tempFile);
    if (!tempFile.IsOpened()) {
        return false;
    }
    tempFile.Write(scriptContent);
    tempFile.Close();

    wxFileName scriptFn(tempPath);
    scriptFn.SetExt("sh");
    wxString scriptPath = scriptFn.GetFullPath();
    if (scriptPath != tempPath && !wxRenameFile(tempPath, scriptPath, true)) {
        wxRemoveFile(tempPath);
        return false;
    }
    if (!wxFileName(scriptPath).SetPermissions(wxS_IRUSR | wxS_IWUSR | wxS_IXUSR)) {
        Tr("Warning: could not set execute permissions on temporary script: " << scriptPath);
    }

    if (!hidden) {
        return RunAsyncInNewWindow(scriptPath, displayIndex, title);
    }

    // Smaller xterm window for background daemon feedback
    int targetCols = 80;
    int targetRows = 10;

    wxRect screenRect = wxDisplay(displayIndex).GetClientArea();
    int posX = screenRect.x + 50;
    int posY = screenRect.y + 50;
    wxString xtermTitle = wxString::Format("%s (Background)", title);

    wxString command = wxString::Format(
        "xterm -bg black -fg white -geometry %dx%d+%d+%d "
        "-T \"%s\" "
        "-xrm \"XTerm*selectToClipboard: true\" "
        "-xrm \"XTerm*vt100.dynamicLayout: true\" "
        "-fa Monospace -fs 12 -hold -e \"%s\"",
        targetCols, targetRows,
        posX, posY,
        xtermTitle,
        scriptPath
    );

    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& scriptPath, int displayIndex, const wxString title)
{
    TrFu;
    wxRect screenRect = wxDisplay(displayIndex).GetClientArea();
    
    int posX = screenRect.x + (screenRect.width - s_DefaultWidth) / 2;
    int posY = screenRect.y + (screenRect.height - s_DefaultHeight) / 2;

    wxString xtermTitle = wxString::Format("%s (Close this window after script is done and back to %s)", title, g_APP_NAME);

    wxString command = wxString::Format(
        "xterm -bg black -fg white -geometry +%d+%d " // <- -bg und -fg too
        "-T \"%s\" "
        "-xrm \"XTerm*vt100.geometry: %dx%d\\nXTerm*selectToClipboard: true\" "
        "-fa Monospace -fs 12 -hold -e \"%s\"",
        posX, posY,
        xtermTitle,
        s_DefaultWidth, s_DefaultHeight,
        scriptPath
    );

    long pid = wxExecute(command, wxEXEC_ASYNC);
    if (pid <= 0) {
        return false;
    }

    auto isStillRunning = [](long processId) {
        wxString procPath = wxString::Format("/proc/%ld", processId);
        return wxDirExists(procPath);
    };

    while (isStillRunning(pid)) {
        wxYieldIfNeeded();
        wxMilliSleep(100);
    }

    return true;
}

#else

bool ProcessRunner::RunAsyncInNewWindow(const wxString& WXUNUSED(scriptPath), int WXUNUSED(displayIndex), const wxString WXUNUSED(title))
{
    TrFu;
    return false;
}

bool ProcessRunner::RunServerAsyncInNewWindow(const wxString& WXUNUSED(scriptContent), int WXUNUSED(displayIndex), const wxString WXUNUSED(title), bool WXUNUSED(hidden))
{
    TrFu;
    return false;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& WXUNUSED(scriptPath), int WXUNUSED(displayIndex), const wxString WXUNUSED(title))
{
    TrFu;
    return false;
}

#endif

