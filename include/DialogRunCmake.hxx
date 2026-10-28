//
// DialogRunCmake.hxx
//

#ifndef DIALOGRUNCMAKE_HXX
#define DIALOGRUNCMAKE_HXX

#include "DialogCmdRunner.hxx"

class DialogRunCmake : public DialogCmdRunner
{
    wxString m_llamaSourceDir;
    wxString m_defOutDir;
    bool m_isCudaEnabled;
#ifdef _WIN32
    wxString m_devEnvPath;
#endif

    void SetupReplacements(ScriptReplacementsList &replacements);
    void LoadScript() override;

    static const wxString s_ScriptRunCmakeFileName;

public:
#ifdef _WIN32
    DialogRunCmake(wxWindow* parent, const wxString& llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled, const wxString& devEnvPath);
#else
    DialogRunCmake(wxWindow* parent, const wxString& llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled);
#endif
    ~DialogRunCmake() = default;
};

#endif // DIALOGRUNCMAKE_HXX