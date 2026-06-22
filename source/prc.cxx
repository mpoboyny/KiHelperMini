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

int ShowGenericMessageBox(const wxString& message, const wxString& caption, int style, wxWindow* parent) {
    wxGenericMessageDialog dlg(parent, message, caption, style);
    return dlg.ShowModal();
}
