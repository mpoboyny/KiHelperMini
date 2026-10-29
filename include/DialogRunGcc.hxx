//
// DialogRunGcc.hxx
//

#ifndef DIALOGRUNGCC_HXX
#define DIALOGRUNGCC_HXX

#include "DialogCmdRunner.hxx"

class DialogRunGcc : public DialogCmdRunner
{
    static const wxString s_ScriptBuildCmakeFileName;

    wxString m_llamaSourceDir;
    bool m_isCudaEnabled;
#ifdef _WIN32
    wxString m_devEnvPath;
#endif
    void SetupReplacements(ScriptReplacementsList &replacements);
    void LoadScript() override;
public:

#ifdef _WIN32
    DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled, const wxString& devEnvPath);
#else
    DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled);
#endif
    ~DialogRunGcc() = default;

private:
    
    
};

#endif // DIALOGRUNGCC_HXX
