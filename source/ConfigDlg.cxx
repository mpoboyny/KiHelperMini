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
    EVT_BUTTON(ID_SHOW_DEF_CONF_FOLDER, ConfigDlg::OnShowDefConfFolder)
    EVT_BUTTON(wxID_OK, ConfigDlg::OnOK)
wxEND_EVENT_TABLE()


ConfigDlg::ConfigDlg(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Configuration Viewer", 
               wxDefaultPosition, wxSize(900, 700), 
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetIcon(wxIcon(edit_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Das Textfeld (StyledTextCtrl)
    m_textCtrl = new wxStyledTextCtrl(this, wxID_ANY);
    SetupXmlHighlighting();
    m_textCtrl->Bind(wxEVT_STC_MARGINCLICK, &ConfigDlg::OnMarginClick, this);
    mainSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 10);

    // 2. Die untere Zeile (Horizontal)
    wxBoxSizer* buttonRowSizer = new wxBoxSizer(wxHORIZONTAL);

    // Linker Button: Config Folder
    wxButton* showConfFolderBtn = new wxButton(this, ID_SHOW_CONF_FOLDER, "Open config folder...");
    showConfFolderBtn->SetBitmap(wxBitmap(open_folder));

    // Optional: Icon-Position anpassen (z.B. links vom Text)
    showConfFolderBtn->SetBitmapPosition(wxLEFT); 


    buttonRowSizer->Add(showConfFolderBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM, 10);

    wxButton* showDefConfFolderBtn = new wxButton(this, ID_SHOW_DEF_CONF_FOLDER, "Open default config folder...");
    showDefConfFolderBtn->SetBitmap(wxBitmap(open_folder));
    showDefConfFolderBtn->SetBitmapPosition(wxLEFT);
    buttonRowSizer->Add(showDefConfFolderBtn, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxBOTTOM, 5);

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
    m_textCtrl->SetLexer(wxSTC_LEX_XML);
    m_textCtrl->SetProperty("fold", "1");
    m_textCtrl->SetProperty("fold.html", "1");
    m_textCtrl->SetProperty("fold.compact", "1");

    wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    m_textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);

    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, monoFont);
    m_textCtrl->StyleClearAll();

    m_textCtrl->StyleSetForeground(wxSTC_H_TAG, wxColour(0, 0, 180));
    m_textCtrl->StyleSetForeground(wxSTC_H_TAGEND, wxColour(0, 0, 180));
    m_textCtrl->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour(128, 64, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
    m_textCtrl->StyleSetForeground(wxSTC_H_ENTITY, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_H_XMLSTART, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_H_XMLEND, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_H_CDATA, wxColour(128, 64, 0));

    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 35);
    m_textCtrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, foreground);

    m_textCtrl->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    m_textCtrl->SetMarginMask(1, wxSTC_MASK_FOLDERS);
    m_textCtrl->SetMarginSensitive(1, true);
    m_textCtrl->SetMarginWidth(1, 16);

    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_PLUS);
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_MINUS); 
    
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_EMPTY);
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_EMPTY);
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
    m_textCtrl->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);

    m_textCtrl->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_textCtrl->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    m_textCtrl->SetCaretForeground(foreground);

    m_textCtrl->MarkerEnableHighlight(false);

    m_textCtrl->SetFoldMarginColour(true, background);
    m_textCtrl->SetFoldMarginHiColour(true, background);

    for (int i = wxSTC_MARKNUM_FOLDEREND; i <= wxSTC_MARKNUM_FOLDEROPEN; ++i) {
        m_textCtrl->MarkerSetForeground(i, foreground);             // Pfeilfarbe (Dunkel)
        m_textCtrl->MarkerSetBackground(i, background);             // Pfeil-Hintergrund (Hell)
        m_textCtrl->MarkerSetBackgroundSelected(i, background);     // Bei Cursorfokus
    }
}

void ConfigDlg::OnMarginClick(wxStyledTextEvent& event)
{
    if (event.GetMargin() != 1) {
        return;
    }

    int line = m_textCtrl->LineFromPosition(event.GetPosition());
    if (m_textCtrl->GetFoldLevel(line) & wxSTC_FOLDLEVELHEADERFLAG) {
        m_textCtrl->ToggleFold(line);
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

void ConfigDlg::SetXmlContent(const wxString& content)
{
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetText(content);
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

void ConfigDlg::OnShowDefConfFolder(wxCommandEvent &event)
{
    wxLaunchDefaultApplication(g_BinDir);
}
