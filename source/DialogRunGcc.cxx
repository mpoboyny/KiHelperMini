//
// DialogRunGcc.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "DialogRunGcc.hxx"

const wxString DialogRunGcc::s_ScriptBuildCmakeFileName = []() -> wxString {
#if defined(_WIN32)
    return "build_cmake.bat";
#else
    return "build_cmake.sh";
#endif
}();

#if defined(_WIN32)
DialogRunGcc::DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled, const wxString& devEnvPath)
    : DialogCmdRunner(parent, "Build with CMake", s_ScriptBuildCmakeFileName, wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_isCudaEnabled(isCudaEnabled)
    , m_devEnvPath(devEnvPath)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}
#else
DialogRunGcc::DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled)
    : DialogCmdRunner(parent, "Build with CMake", s_ScriptBuildCmakeFileName, wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_isCudaEnabled(isCudaEnabled)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}
#endif

void DialogRunGcc::SetupReplacements(ScriptReplacementsList &replacements)
{
    TrFu;
    TrStr(m_llamaSourceDir);

    wxFileName llamaBuildFn;
    llamaBuildFn.AssignDir(m_llamaSourceDir);
    llamaBuildFn.AppendDir("llama.cpp");
    llamaBuildFn.AppendDir("llama.cpp-master");
    wxString llamaSrcDir = llamaBuildFn.GetPathWithSep();
    wxString llamaCudaFlag = m_isCudaEnabled ? "ON" : "OFF";
#ifdef _WIN32
    replacements = {
        { "<!-- llama_src_dir -->", llamaSrcDir },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag },
        { "<!-- vs_dev_cmd_path -->", m_devEnvPath }
    };
#else
    replacements = {
        { "<!-- llama_src_dir -->", llamaSrcDir },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag }
    };
#endif
}

void DialogRunGcc::LoadScript()
{
    ScriptReplacementsList replacements;
    SetupReplacements(replacements);
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptBuildCmakePath, replacements);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
