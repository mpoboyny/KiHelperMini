//
// DialogRunOpencode.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "ConfigFile.hxx"
#include "DialogRunOpencode.hxx"
#include "DialogOpencodeJson.hxx"

DialogRunOpencode::DialogRunOpencode(wxWindow* parent, const ConfigFile &confFile)
    : DialogCmdRunner(parent, "Run OpenCode", wxFileName(g_ScriptRunOpenCodePath).GetFullName(), wxSize(1000, 900), false)
    , m_ConfFile(confFile)
{
    TrFu;
    SetupHighlighting();
    LoadScript();
    AddCustomButtons();
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

void DialogRunOpencode::AddCustomButtons()
{
    wxButton* showConfigBtn = new wxButton(this, wxID_ANY, "Show opencode.json ...");
    showConfigBtn->Bind(wxEVT_BUTTON, &DialogRunOpencode::OnShowOpenCodeConfig, this);
    AddCustomButton(showConfigBtn);
}

void DialogRunOpencode::OnShowOpenCodeConfig(wxCommandEvent& event)
{
    DialogOpencodeJson dialog(this, m_ConfFile);
    dialog.ShowModal();
}
