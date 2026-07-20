//
// MainRunChatPanel.cxx
//

#include "prc.hxx"
#include "ConfigFile.hxx"
#include "MainRunChatPanel.hxx"

/*static*/
const wxString CMainRunChatPanel::s_ChatFileName = "llama-cli";

CMainRunChatPanel::CMainRunChatPanel(wxWindow* parent, const ConfigFile &confFile)
    : wxPanel(parent, wxID_ANY)
{
    TrFu;

    // 1. Main vertical sizer for the entire panel
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 2. Create the "Run chat" StaticBox container
    wxStaticBox* runGroupBox = new wxStaticBox(this, wxID_ANY, "Run chat");
    wxStaticBoxSizer* runSizer = new wxStaticBoxSizer(runGroupBox, wxVERTICAL);

    // 3. File path selection row
    wxBoxSizer* fileRowSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* labelFilePath = new wxStaticText(runGroupBox, wxID_ANY, "File path:");

    // Construct the default binary path plattform-independently
    wxFileName targetFn(confFile.GetLLamaBinPath());
    targetFn.AppendDir("bin");
    targetFn.SetFullName(s_ChatFileName);

    m_textFilePath = new wxTextCtrl(runGroupBox, wxID_ANY, targetFn.GetFullPath(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    wxButton* btnSelectFile = new wxButton(runGroupBox, ID_SELECT_CHAT_FILE, "Select file");

    fileRowSizer->Add(labelFilePath, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    fileRowSizer->Add(m_textFilePath, 1, wxEXPAND | wxRIGHT, 5); 
    fileRowSizer->Add(btnSelectFile, 0, wxALIGN_CENTER_VERTICAL);

    // 4. Parameters label row
    wxBoxSizer* paramRowSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* labelParams = new wxStaticText(runGroupBox, wxID_ANY, "Parameters:");
    paramRowSizer->Add(labelParams, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    m_textChatParams = new wxTextCtrl(runGroupBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
    paramRowSizer->Add(m_textChatParams, 1, wxEXPAND | wxRIGHT, 5);

    for (const auto& param : confFile.GetChatParameters(confFile.GetCurrentSysName())) {
        wxString paramText = param.Name + " " + param.Value;
        m_textChatParams->AppendText(paramText + " ");
    }

    // 5. Add inner row sizers to the runSizer FIRST
    runSizer->Add(fileRowSizer, 0, wxEXPAND | wxALL, 10);
    runSizer->Add(paramRowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    
    // 6. Add the main container to the panel's layout LAST
    mainSizer->Add(runSizer, 1, wxEXPAND | wxALL, 10);
    SetSizer(mainSizer);
    
    // Event Bindings
    Bind(wxEVT_TOOL, &CMainRunChatPanel::OnRunSample, this, ID_RUN_SAMPLE);
    Bind(wxEVT_BUTTON, &CMainRunChatPanel::OnSelectFile, this, ID_SELECT_CHAT_FILE);

    Layout();
}


void CMainRunChatPanel::OnRunSample(wxCommandEvent& WXUNUSED(event))
{
    ShowGenericMessageBox("Sample execute action.", "Execute", wxOK | wxICON_INFORMATION, this);
}

void CMainRunChatPanel::OnSelectFile(wxCommandEvent& event)
{
    // TrFu;
    wxString currentPath = m_textFilePath->GetValue();
    wxString defaultDir = "";
    wxString defaultFile = "";

    // 1. If a path is already set, split it into directory and filename
    if (!currentPath.IsEmpty())
    {
        wxFileName fn(currentPath);
        // Explicit format with trailing separator
        defaultDir = fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);      
        defaultFile = fn.GetFullName(); 
    }

    // TrStr(defaultDir);
    // TrStr(defaultFile);

    // 2. Create the file dialog
    wxFileDialog openFileDialog(
        this, 
        "Select llama-cli binary", 
        defaultDir,   
        defaultFile,  
        "All files (*.*)|*.*", 
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    // 3. CRITICAL GTK FIX: Explicitly force the file selection right before displaying
    if (!defaultDir.IsEmpty())
    {
        openFileDialog.SetDirectory(defaultDir);
    }
    if (!defaultFile.IsEmpty())
    {
        openFileDialog.SetFilename(defaultFile); // Forces GTK to highlight the target file
    }

    // 4. Show the dialog
    if (openFileDialog.ShowModal() == wxID_CANCEL)
    {
        return; 
    }

    if (m_textFilePath) 
    {
        // Displays the full absolute path as text in your text control
        m_textFilePath->SetValue(openFileDialog.GetPath());
    }
}
