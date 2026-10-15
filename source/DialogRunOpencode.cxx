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

void DialogRunOpencode::OnDialogDoIt(wxCommandEvent& event)
{
    const wxString projectFolder = m_ConfFile.GetOpenCodeDefProjectFolder();
    if (projectFolder.IsEmpty()) {
        ShowGenericMessageBox("No OpenCode project folder is configured.", "Run OpenCode", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!wxFileName::DirExists(projectFolder) &&
        !wxFileName::Mkdir(projectFolder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
        ShowGenericMessageBox("Failed to create the OpenCode project folder:\n" + projectFolder,
                              "Run OpenCode", wxOK | wxICON_ERROR, this);
        return;
    }

    DialogOpencodeJson jsonDialog(this, m_ConfFile);
    wxFileName jsonFile;
    jsonFile.AssignDir(projectFolder);
    jsonFile.SetFullName("opencode.json");

    wxFile file(jsonFile.GetFullPath(), wxFile::write);
    if (!file.IsOpened()) {
        ShowGenericMessageBox("Failed to save OpenCode configuration:\n" + jsonFile.GetFullPath(),
                              "Run OpenCode", wxOK | wxICON_ERROR, this);
        return;
    }

    file.Write(jsonDialog.GetJsonContent(), wxConvUTF8);
    file.Close();

    DialogCmdRunner::OnDialogDoIt(event);
}
