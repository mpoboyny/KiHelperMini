#include "prc.hxx"

namespace frameMP 
{
}; // namespace frameMP

const int g_DefaultTxtSize = 10;
const wchar_t *g_DefaultTxtFontName = L"Sans";

const wchar_t *g_APP_NAME = L"KiHelper-Mini";
const char *g_APP_NAME_A = "KiHelper-Mini";

extern const wchar_t           *g_APP_NAME;
extern const char              *g_APP_NAME_A;

wxString g_WorkDir;

const wxString g_ConfDir = []() {
    wxFileName fn;
    fn.AssignDir(wxStandardPaths::Get().GetDocumentsDir());
    fn.AppendDir(g_APP_NAME);
    if (!fn.DirExists()) {
        fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    return fn.GetPathWithSep();
}();

const wxString g_ConfFileName = L"config.xml";

const wxString g_ConfFile = []() {
    wxString targetPath = g_ConfDir + g_ConfFileName;
    wxFileName targetFn(targetPath);
    if (!targetFn.FileExists()) {
        wxFileName exeFn(wxStandardPaths::Get().GetExecutablePath());
        wxString srcPath = exeFn.GetPathWithSep() + g_ConfFileName;
        wxFileName srcFn(srcPath);
        if (srcFn.FileExists()) {
            wxCopyFile(srcPath, targetPath);
        }
    }
    return targetPath;
}();

const wxString g_ScriptDir = []() -> wxString {
    wxFileName exeFn(wxStandardPaths::Get().GetExecutablePath());
    wxFileName scriptsFn;
    scriptsFn.AssignDir(exeFn.GetPath());
    scriptsFn.AppendDir("scripts");

    if (!scriptsFn.DirExists()) {
        return wxEmptyString;
    }

    return scriptsFn.GetPathWithSep();
}();

const wxString g_ScriptExtraLlamaPath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "extract_llama.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "extract_llama.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptBuildCmakePath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "build_cmake.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "build_cmake.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptRunCmakePath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "run_cmake.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "run_cmake.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptRunLlamaPath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "run_llama.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "run_llama.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

void SetWorkingDir()
{
    TrFu;
    std::filesystem::path workDirPath;

#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len != 0 && len < MAX_PATH) {
        workDirPath = std::filesystem::path(buffer).parent_path();
    }
#else
    char buffer[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        workDirPath = std::filesystem::path(buffer).parent_path();
    }
#endif

    if (workDirPath.empty()) {
        workDirPath = std::filesystem::current_path();
    }

    std::error_code ec;
    std::filesystem::current_path(workDirPath, ec);
    
    std::filesystem::path currentPath = std::filesystem::current_path(ec);
    if (!ec) {
        g_WorkDir = wxString::FromUTF8(currentPath.string());
        if (!g_WorkDir.empty()) {
            wxFileName workFn(g_WorkDir);
            if (workFn.DirExists()) {
                g_WorkDir = workFn.GetPathWithSep();
            }
        }
    }
    TrStr(g_WorkDir);
}

int ShowGenericMessageBox(const wxString &message, const wxString &caption, int style, wxWindow *parent)
{
    wxGenericMessageDialog dlg(parent, message, caption, style);
    return dlg.ShowModal();
}

bool RemoveDirectoryContents(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened()) {
        return false;
    }

    wxString name;
    bool cont = dir.GetFirst(&name, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);
    while (cont) {
        wxString item = path + wxFileName::GetPathSeparator() + name;
        if (wxDirExists(item)) {
            if (!RemoveDirectoryContents(item) || !wxRmdir(item)) {
                return false;
            }
        } else {
            if (!wxRemoveFile(item)) {
                return false;
            }
        }
        cont = dir.GetNext(&name);
    }

    return true;
}

wxColour g_ColorLightGreen = wxColour(220, 245, 220); // Light green
