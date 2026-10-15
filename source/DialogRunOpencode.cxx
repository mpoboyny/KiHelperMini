//
// DialogRunOpencode.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "DialogRunOpencode.hxx"

DialogRunOpencode::DialogRunOpencode(wxWindow* parent)
    : DialogCmdRunner(parent, "Run OpenCode", wxFileName(g_ScriptRunOpenCodePath).GetFullName(), wxSize(800, 600))
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}

void DialogRunOpencode::LoadScript()
{
    ScriptReplacementsList replacements;
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunOpenCodePath, replacements);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}