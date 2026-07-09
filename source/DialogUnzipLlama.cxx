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

    SetupHighlighting();
    LoadScript();
}

void DialogUnzipLlama::SetupReplacements()
{
    // m_llamaSourceDir
    m_scriptReplacements = {
        { "<!-- llama.cpp-master.zip -->", g_WorkDir }
    };
}

void DialogUnzipLlama::LoadScript()
{
    wxString content;
    if (g_ScriptExtraLlamaPath.IsEmpty()) {
        content = "Script not found: scripts/extract_llama.sh";
    } else {
        wxTextFile file(g_ScriptExtraLlamaPath);
        if (!file.Open()) {
            content = wxString::Format("Cannot open script file:\n%s", g_ScriptExtraLlamaPath);
        } else {
            for (size_t i = 0; i < file.GetLineCount(); ++i) {
                content += file.GetLine(i);
                content += "\n";
            }
        }
    }

    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
