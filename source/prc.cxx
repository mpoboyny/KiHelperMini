#include "prc.hxx"

namespace frameMP 
{
}; // namespace frameMP

const int g_DefaultTxtSize = 10;
const wchar_t *g_DefaultTxtFontName = L"Sans";

const wchar_t *g_APP_NAME = L"KiHelper-Mini";

const wxString g_ConfDir = []() {
    wxFileName fn;
    fn.AssignDir(wxStandardPaths::Get().GetDocumentsDir());
    fn.AppendDir(wxString(L".") + g_APP_NAME);
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
    const wxString scriptFileName = "extract_llama.sh";
    #if defined(_WIN32)
        const wxString scriptFileName = "extract_llama.bat";
    #endif

    wxFileName scriptsFn(g_ScriptDir);
    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }
    scriptsFn.AppendDir(wxEmptyString);
    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

int ShowGenericMessageBox(const wxString& message, const wxString& caption, int style, wxWindow* parent) {
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
