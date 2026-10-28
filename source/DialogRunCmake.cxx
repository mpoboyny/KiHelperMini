//
// DialogRunCmake.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "DialogRunCmake.hxx"

const wxString DialogRunCmake::s_ScriptRunCmakeFileName = []() -> wxString {
#if defined(__gnu_linux__)
    return "run_cmake.sh";
#elif defined(_WIN32)
    return "run_cmake.bat";
#endif
}();

#ifdef _WIN32
DialogRunCmake::DialogRunCmake(wxWindow* parent, const wxString& llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled, const wxString& devEnvPath)
    : DialogCmdRunner(parent, "Run CMake", s_ScriptRunCmakeFileName, wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_defOutDir(defOutDir)
    , m_isCudaEnabled(isCudaEnabled)
    , m_devEnvPath(devEnvPath)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}
#else
DialogRunCmake::DialogRunCmake(wxWindow *parent, const wxString &llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled)
    : DialogCmdRunner(parent, "Run CMake", s_ScriptRunCmakeFileName, wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_defOutDir(defOutDir)
    , m_isCudaEnabled(isCudaEnabled)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}
#endif

void DialogRunCmake::SetupReplacements(ScriptReplacementsList &replacements)
{
    TrFu;
    TrStr(m_llamaSourceDir);
    wxFileName llamaSrcFn;
    llamaSrcFn.AssignDir(m_llamaSourceDir);
    llamaSrcFn.AppendDir("llama.cpp");
    llamaSrcFn.AppendDir("llama.cpp-master");
    wxString llamaSrc = llamaSrcFn.GetPathWithSep();
    wxString llamaCudaFlag = m_isCudaEnabled ? "ON" : "OFF";
    wxFileName llamaBinFn;
    llamaBinFn.AssignDir(m_defOutDir);
    llamaBinFn.AppendDir(wxString((m_isCudaEnabled ? "withCuda" : "noCuda")));
    wxString llamaBin = llamaBinFn.GetPathWithSep();

#ifdef _WIN32
    replacements = {
        { "<!-- llama_src_dir -->", llamaSrc },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag },
        { "<!-- llama_out_dir -->", llamaBin },
        { "<!-- vs_dev_cmd_path -->", m_devEnvPath }
    };
#else
    replacements = {
        { "<!-- llama_src_dir -->", llamaSrc },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag },
        { "<!-- llama_out_dir -->", llamaBin }
    };
#endif
}

void DialogRunCmake::LoadScript()
{
    ScriptReplacementsList replacements;
    SetupReplacements(replacements);
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunCmakePath, replacements);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
