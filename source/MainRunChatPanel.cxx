//
// MainRunChatPanel.cxx
//

#include "prc.hxx"
#include "MainRunChatPanel.hxx"

/*static*/
const wxString CMainRunChatPanel::s_ChatFileName = "llama-cli";

CMainRunChatPanel::CMainRunChatPanel(wxWindow* parent, wxString defDirChatFileDir)
    : wxPanel(parent, wxID_ANY)
{
    TrFu;

    // 1. Haupt-Sizer für das Panel (Vertikal)
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 2. Die StaticBox "Run chat" erstellen (wird nun der Hauptcontainer)
    wxStaticBox* runGroupBox = new wxStaticBox(this, wxID_ANY, "Run chat");
    wxStaticBoxSizer* runSizer = new wxStaticBoxSizer(runGroupBox, wxVERTICAL);

    // 3. Horizontale Zeile für die Dateiauswahl INNERHALB der Box erstellen
    wxBoxSizer* fileRowSizer = new wxBoxSizer(wxHORIZONTAL);

    // Wichtig: 'runGroupBox' als Parent übergeben, damit die Controls IN der Box liegen
    wxStaticText* labelFilePath = new wxStaticText(runGroupBox, wxID_ANY, "File path:");

    wxFileName targetFn(defDirChatFileDir);
    targetFn.AppendDir("bin");
    targetFn.SetFullName(s_ChatFileName);
    m_textFilePath = new wxTextCtrl(runGroupBox, wxID_ANY, targetFn.GetFullPath(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    wxButton* btnSelectFile = new wxButton(runGroupBox, ID_SELECT_CHAT_FILE, "Select file");

    // Elemente in die horizontale Zeile einfügen
    fileRowSizer->Add(labelFilePath, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    fileRowSizer->Add(m_textFilePath, 1, wxEXPAND | wxRIGHT, 5); // Streckt das Textfeld automatisch
    fileRowSizer->Add(btnSelectFile, 0, wxALIGN_CENTER_VERTICAL);

    // Die Dateizeile ganz oben in den runSizer der StaticBox packen
    runSizer->Add(fileRowSizer, 0, wxEXPAND | wxALL, 10);

    // HIER kannst du später weitere Chat-Elemente direkt unter die Dateizeile einfügen:
    // runSizer->Add(m_chatHistory, 1, wxEXPAND | wxALL, 10);

    // Den runSizer in den mainSizer des Panels packen (nimmt den restlichen Platz ein)
    mainSizer->Add(runSizer, 1, wxEXPAND | wxALL, 10);

    // Sizer dem Panel zuweisen
    SetSizer(mainSizer);
    
    // Event-Bindings
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
