//
// DialogCmdRunner.cxx
//

#include "prc.hxx"
#include "DialogCmdRunner.hxx"
#include "ProcessRunner.hxx"

#include <chrono>
#include <thread>

DialogCmdRunner::DialogCmdRunner(wxWindow *parent,
                                 const wxString &dialogTitle,
                                 const wxString &headerText,
                                 const wxSize &dialogSize,
                                 bool runScriptSync)
    : wxDialog(parent, wxID_ANY, dialogTitle, wxDefaultPosition, dialogSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_ScriptFileName(headerText)
    , m_RunScriptSync(runScriptSync)
    , m_textCtrl(nullptr)
    , m_buttonRow(nullptr)
{
    TrFu;
    TrStr(m_ScriptFileName);
    
    int dotPos = m_ScriptFileName.Find('.', true); 
    if (dotPos != wxNOT_FOUND) {
        m_ScriptFileExtension = m_ScriptFileName.Mid(dotPos + 1);
    }

    TrStr(m_ScriptFileExtension);

    SetClientSize(dialogSize);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* headerRow = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, m_ScriptFileName);
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    headerRow->Add(title, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 16);

    wxStaticText* workDir = new wxStaticText(this, wxID_ANY, "Work directory: " + g_WorkDir);
    headerRow->Add(workDir, 0, wxALIGN_CENTER_VERTICAL);

    top->Add(headerRow, 0, wxALL, 10);

    top->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    m_textCtrl = new wxStyledTextCtrl(this, wxID_ANY);
    top->Add(m_textCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    m_buttonRow = new wxBoxSizer(wxHORIZONTAL);
    m_buttonRow->AddStretchSpacer(1);

    wxButton* closeBtn = new wxButton(this, wxID_CANCEL, "Close");
    m_buttonRow->Add(closeBtn, 0, wxALL, 10);
    wxButton* doItBtn = new wxButton(this, wxID_OK, "Do it");
    m_buttonRow->Add(doItBtn, 0, wxALL, 10);
    Bind(wxEVT_BUTTON, &DialogCmdRunner::OnDialogDoIt, this, wxID_OK);
    top->Add(m_buttonRow, 0, wxEXPAND);

    SetSizer(top);
    SetMinSize(dialogSize);
    top->SetMinSize(dialogSize);
    top->SetSizeHints(this);

    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        EndModal(wxID_CANCEL);
    }, wxID_CANCEL);

    CentreOnParent();
}

#ifdef _WIN32

void DialogCmdRunner::SetupHighlighting()
{
    m_textCtrl->SetLexer(wxSTC_LEX_BASH);
    wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    m_textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);

    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    for (int i = 0; i <= wxSTC_STYLE_MAX; ++i) {
        m_textCtrl->StyleSetFont(i, monoFont);
    }
    
    m_textCtrl->StyleClearAll();

    m_textCtrl->StyleSetForeground(wxSTC_SH_COMMENTLINE, wxColour(0, 128, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_NUMBER, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_SH_WORD, wxColour(0, 0, 180));
    m_textCtrl->StyleSetBold(wxSTC_SH_WORD, true);
    m_textCtrl->StyleSetForeground(wxSTC_SH_STRING, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_CHARACTER, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_OPERATOR, foreground);
    m_textCtrl->StyleSetForeground(wxSTC_SH_SCALAR, wxColour(0, 96, 160));
    m_textCtrl->StyleSetForeground(wxSTC_SH_PARAM, wxColour(128, 64, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_BACKTICKS, wxColour(128, 0, 64));

    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 40);
    m_textCtrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, foreground);

    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetWrapMode(wxSTC_WRAP_NONE);
    m_textCtrl->SetTabWidth(4);

    wxString keywords = "if then else elif fi do done for while case esac in function select "
                        "until break continue return exit local export readonly declare typeset "
                        "shift source dot echo set mkdir cd not exist error";
                        
    m_textCtrl->SetKeyWords(0, keywords);
    m_textCtrl->Colourise(0, -1);
}


#else

void DialogCmdRunner::SetupHighlighting()
{
    m_textCtrl->SetLexer(wxSTC_LEX_BASH);
    m_textCtrl->StyleClearAll();

    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    for (int i = 0; i <= wxSTC_STYLE_MAX; ++i) {
        m_textCtrl->StyleSetFont(i, monoFont);
    }

    m_textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColour(250, 250, 250));
    m_textCtrl->StyleSetForeground(wxSTC_SH_COMMENTLINE, wxColour(0, 128, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_NUMBER, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_SH_WORD, wxColour(0, 0, 180));
    m_textCtrl->StyleSetForeground(wxSTC_SH_STRING, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_CHARACTER, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_OPERATOR, wxColour(0, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_SCALAR, wxColour(0, 96, 160));
    m_textCtrl->StyleSetForeground(wxSTC_SH_PARAM, wxColour(128, 64, 0));
    m_textCtrl->StyleSetForeground(wxSTC_SH_BACKTICKS, wxColour(128, 0, 64));

    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 40);
    m_textCtrl->SetReadOnly(false);

    m_textCtrl->SetKeyWords(0, "if then else elif fi do done for while case esac in function select until break continue return exit local export readonly declare typeset shift source dot");
    m_textCtrl->SetWrapMode(wxSTC_WRAP_NONE);
    m_textCtrl->SetTabWidth(4);
}

#endif

void DialogCmdRunner::AddCustomButton(wxButton* button)
{
    m_buttonRow->Insert(0, button, 0, wxALL, 10);
    GetSizer()->Layout();
}

void DialogCmdRunner::OnDialogDoIt(wxCommandEvent& event)
{
    TrFu;

    if (!m_textCtrl) {
        ShowGenericMessageBox("No script content available.", "Run script", wxOK | wxICON_ERROR, this);
        return;
    }

    wxString scriptContent = m_textCtrl->GetText();
    wxFile tempFile;
    wxString tempPath = wxFileName::CreateTempFileName("DialogCmdRunner-", &tempFile);
    if (tempPath.IsEmpty() || !tempFile.IsOpened()) {
        ShowGenericMessageBox("Failed to create a temporary script file.", "Run script", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!tempFile.Write(scriptContent, wxConvUTF8) || !tempFile.Flush()) {
        tempFile.Close();
        wxRemoveFile(tempPath);
        ShowGenericMessageBox("Failed to write the temporary script file.", "Run script", wxOK | wxICON_ERROR, this);
        return;
    }

    tempFile.Close();

    wxFileName scriptFn(tempPath);

    if (!m_ScriptFileExtension.IsEmpty())
        scriptFn.SetExt(m_ScriptFileExtension);

    wxString scriptPath = scriptFn.GetFullPath();
    if (scriptPath != tempPath && !wxRenameFile(tempPath, scriptPath, true)) {
        wxRemoveFile(tempPath);
        ShowGenericMessageBox("Failed to prepare the temporary script file.", "Run script", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!wxFileName(scriptPath).SetPermissions(wxS_IRUSR | wxS_IWUSR | wxS_IXUSR)) {
        Tr("Warning: could not set execute permissions on temporary script: " << scriptPath);
    }

    ProcessRunner runner;
    bool scriptStarted;
    if (m_RunScriptSync) {
        scriptStarted = runner.RunSyncInNewWindow(scriptPath, wxDisplay::GetFromWindow(this), m_ScriptFileName);
    }
    else {
        scriptStarted = runner.RunServerAsyncInNewWindow(scriptPath, wxDisplay::GetFromWindow(this), m_ScriptFileName);
    }

    if (!scriptStarted) {
        wxRemoveFile(scriptPath);
        ShowGenericMessageBox("Failed to launch the script in a new window.", "Run script", wxOK | wxICON_ERROR, this);
        return;
    }

    std::thread([scriptPath]() {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        wxRemoveFile(scriptPath);
    }).detach();

    if (IsModal()) {
        EndModal(wxID_OK);
    }
    else {
        Close();
    }
}
