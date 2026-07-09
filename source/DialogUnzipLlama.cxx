//
// DialogUnzipLlama.cxx
//

#include "prc.hxx"
#include "DialogUnzipLlama.hxx"

DialogUnzipLlama::DialogUnzipLlama(wxWindow* parent, const wxString &llamaSorceDir)
    : DialogCmdRunner(parent, "Unzip llama.cpp", "extract_llama.sh", wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
{
    TrFu;
    SetupReplacements();
    SetupHighlighting();
    LoadScript();
}

void DialogUnzipLlama::SetupReplacements()
{
    m_scriptReplacements = {
        { "<! llama_root_dir -->", m_llamaSourceDir }
    };
}

void DialogUnzipLlama::LoadScript()
{
    wxString content = GetScriptContentWithReplacements(g_ScriptExtraLlamaPath);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
