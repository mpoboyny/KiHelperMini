//
// DialogRunOpencode.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "ConfigFile.hxx"
#include "DialogRunOpencode.hxx"

DialogRunOpencode::DialogRunOpencode(wxWindow* parent, const ConfigFile &confFile)
    : DialogCmdRunner(parent, "Run OpenCode", wxFileName(g_ScriptRunOpenCodePath).GetFullName(), wxSize(800, 600))
    , m_ConfFile(confFile)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
}

void DialogRunOpencode::SetupReplacements(ScriptReplacementsList &replacements)
{
    TrFu;
    replacements = {
        { "<!-- opencode_prj_dir -->", m_ConfFile.GetOpenCodeDefProjectFolder()}
    };
}

void DialogRunOpencode::LoadScript()
{
    ScriptReplacementsList replacements;
    SetupReplacements(replacements);
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptRunOpenCodePath, replacements);
    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetSavePoint();
}
