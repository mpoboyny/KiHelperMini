//
// DialogRunCmake.cxx
//

#include "prc.hxx"
#include "DialogRunCmake.hxx"

DialogRunCmake::DialogRunCmake(wxWindow *parent, const wxString &llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled)
    : DialogCmdRunner(parent, "Run CMake", "run_cmake.sh", wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
    , m_defOutDir(defOutDir)
    , m_isCudaEnabled(isCudaEnabled)
{
    TrFu;
    SetupReplacements();
    SetupHighlighting();
    LoadScript();
}

void DialogRunCmake::SetupReplacements()
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


    m_scriptReplacements = {
        { "<!-- llama_src_dir -->", llamaSrc },
        { "<!-- llama_cuda_flag -->", llamaCudaFlag },
        { "<!-- llama_out_dir -->", llamaBin }
    };
}

void DialogRunCmake::LoadScript()
{
    wxString content = GetScriptContentWithReplacements(g_ScriptRunCmakePath);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}