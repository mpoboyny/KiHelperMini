//
// DialogRunGcc.cxx
//

#include "prc.hxx"
#include "DialogRunGcc.hxx"

DialogRunGcc::DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled)
    : DialogCmdRunner(parent, "Build with CMake", "build_cmake.sh", wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_isCudaEnabled(isCudaEnabled)
{
    TrFu;
    SetupReplacements();
    SetupHighlighting();
    LoadScript();
}

void DialogRunGcc::SetupReplacements()
{
    TrFu;
    TrStr(m_llamaSourceDir);

    wxFileName llamaBuildFn;
    llamaBuildFn.AssignDir(m_llamaSourceDir);
    llamaBuildFn.AppendDir("llama.cpp");
    llamaBuildFn.AppendDir("llama.cpp-master");
    wxString llamaSrcDir = llamaBuildFn.GetPathWithSep();
    wxString llamaCudaFlag = m_isCudaEnabled ? "ON" : "OFF";

    m_scriptReplacements = {
        { "<!-- llama_src_dir -->", llamaSrcDir },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag }
    };
}

void DialogRunGcc::LoadScript()
{
    wxString content = GetScriptContentWithReplacements(g_ScriptBuildCmakePath);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}