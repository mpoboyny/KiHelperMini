//
// DialogUnzipLlama.cxx
//

#include "prc.hxx"
#include "DialogUnzipLlama.hxx"
#include "ScriptHandler.hxx"

DialogUnzipLlama::DialogUnzipLlama(wxWindow* parent, const wxString &llamaSorceDir)
    : DialogCmdRunner(parent, "Unzip llama.cpp", wxFileName(g_ScriptExtraLlamaPath).GetFullName(), wxSize(800, 600))
    , m_llamaSourceDir(llamaSorceDir)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}

void DialogUnzipLlama::LoadScript()
{
    ScriptReplacementsList replacements = {
        { "<! llama_root_dir -->", m_llamaSourceDir }
    };
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptExtraLlamaPath, replacements);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
