//
// ConfigDlg.cxx
//

#include "prc.hxx"
#include "ConfigDlg.hxx"
#include "ConfigFile.hxx"
#include "../resources/open_folder.xpm"
#include "../resources/edit-4.xpm"

wxBEGIN_EVENT_TABLE(ConfigDlg, wxDialog)
    EVT_BUTTON(ID_SHOW_CONF_FOLDER, ConfigDlg::OnShowConfFolder)
    EVT_BUTTON(wxID_OK, ConfigDlg::OnOK)
wxEND_EVENT_TABLE()


ConfigDlg::ConfigDlg(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Configuration Viewer", 
               wxDefaultPosition, wxSize(700, 500), 
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetIcon(wxIcon(edit_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Das Textfeld (StyledTextCtrl)
    m_textCtrl = new wxStyledTextCtrl(this, wxID_ANY);
    SetupXmlHighlighting();
    mainSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 10);

    // 2. Die untere Zeile (Horizontal)
    wxBoxSizer* buttonRowSizer = new wxBoxSizer(wxHORIZONTAL);

    // Linker Button: Config Folder
    wxButton* showConfFolderBtn = new wxButton(this, ID_SHOW_CONF_FOLDER, "Open config folder...");
    showConfFolderBtn->SetBitmap(wxBitmap(open_folder));

    // Optional: Icon-Position anpassen (z.B. links vom Text)
    showConfFolderBtn->SetBitmapPosition(wxLEFT); 


    buttonRowSizer->Add(showConfFolderBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM, 10);

    // Der Spacer drückt alles nachfolgende nach rechts
    buttonRowSizer->AddStretchSpacer(1);

    // Rechter Button: OK (Manuell erstellt statt CreateButtonSizer)
    wxButton* okBtn = new wxButton(this, wxID_OK, "Save and use");
    okBtn->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_BUTTON));
    okBtn->SetDefault(); // Macht den Button zum Standard (Enter-Taste)
    buttonRowSizer->Add(okBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 10);

    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Discard changes and exit");
    cancelBtn->SetBitmap(wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON));
    buttonRowSizer->Add(cancelBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 10);

    // 3. Zeile zum Haupt-Sizer hinzufügen
    mainSizer->Add(buttonRowSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
    
    // WICHTIG: Erst laden, wenn das UI steht
    LoadXmlContent();
}

void ConfigDlg::SetupXmlHighlighting()
{
    // Set XML Lexer
    m_textCtrl->SetLexer(wxSTC_LEX_XML);

    // Style colors (Example: Dark blue for tags, Red for attributes)
    m_textCtrl->StyleSetForeground(wxSTC_H_TAG, wxColour(0, 0, 150));
    m_textCtrl->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour(150, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_VALUE, wxColour(0, 150, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_COMMENT, wxColour(128, 128, 128));
    
    // Enable line numbers
    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 35);

    // Set font
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    for (int i = 0; i <= wxSTC_STYLE_MAX; ++i) {
        m_textCtrl->StyleSetFont(i, monoFont);
    }
}

void ConfigDlg::LoadXmlContent()
{
    ConfigFile conf;
    m_originalContent = conf;
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetText(m_originalContent);
    m_textCtrl->SetSavePoint();
}

bool ConfigDlg::IsModified() const
{
    return m_textCtrl->GetModify();
}

void ConfigDlg::SaveConfig()
{
    wxFile file(g_ConfFile, wxFile::write);
    if (file.IsOpened()) {
        file.Write(m_textCtrl->GetText(), wxConvUTF8);
        file.Close();
    }
}

void ConfigDlg::OnOK(wxCommandEvent &event)
{
    TrFu;
    if (IsModified()) {
        wxString currentXml = m_textCtrl->GetText();
        wxStringInputStream stream(currentXml);
        wxXmlDocument doc;
        wxXmlParseError err;
        
        wxLogNull silence;
        if (!doc.Load(stream, wxXMLDOC_NONE, &err)) {
            wxString errorMsg;
            if (err.line != 0) {
                errorMsg = wxString::Format("XML Error: %s at line %d, col %d", err.message, err.line, err.column);
            } else {
                errorMsg = "Error: Invalid XML structure.";
            }
            wxMessageBox(errorMsg, "XML Validation Error", wxOK | wxICON_ERROR, this);
            return; // Do nothing, keep dialog open
        }
    }
    event.Skip();
}

void ConfigDlg::OnShowConfFolder(wxCommandEvent &event)
{
    wxLaunchDefaultApplication(g_ConfDir);
}
