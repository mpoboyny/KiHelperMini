//
// DialogPresetIni.cxx
//

#include "prc.hxx"
#include "DialogPresetIni.hxx"
#include "../resources/app.xpm"
#include "../resources/edit-4.xpm"

DialogPresetIni::DialogPresetIni(wxWindow* parent, const wxString& presetIni)
    : wxDialog(parent, wxID_ANY, "preset.ini", wxDefaultPosition, wxSize(800, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_textCtrl(nullptr)
    , m_presetIni(presetIni)
{
    SetIcon(wxIcon(app_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_textCtrl = new wxStyledTextCtrl(this, wxID_ANY);
    SetupIniHighlighting();
    m_textCtrl->SetText(m_presetIni);
    m_textCtrl->SetReadOnly(true);
    mainSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* copyButton = new wxButton(this, wxID_ANY, "Copy to clipboard");
    copyButton->SetBitmap(wxBitmap(edit_xpm));
    copyButton->SetBitmapPosition(wxLEFT);
    buttonSizer->Add(copyButton, 0, wxLEFT | wxBOTTOM, 10);
    buttonSizer->AddStretchSpacer(1);

    wxButton* okButton = new wxButton(this, wxID_OK, "OK");
    okButton->SetBitmapPosition(wxLEFT);
    okButton->SetDefault();
    buttonSizer->Add(okButton, 0, wxRIGHT | wxBOTTOM, 10);
    mainSizer->Add(buttonSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
    SetMinSize(wxSize(600, 400));
    CentreOnParent();

    Bind(wxEVT_BUTTON, &DialogPresetIni::OnCopyToClipboard, this, copyButton->GetId());
}

void DialogPresetIni::SetupIniHighlighting()
{
    m_textCtrl->SetLexer(wxSTC_LEX_PROPERTIES);

    wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    m_textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);
    m_textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, monoFont);
    m_textCtrl->StyleClearAll();

    m_textCtrl->StyleSetForeground(wxSTC_PROPS_COMMENT, wxColour(0, 128, 0));
    m_textCtrl->StyleSetForeground(wxSTC_PROPS_SECTION, wxColour(0, 0, 180));
    m_textCtrl->StyleSetBold(wxSTC_PROPS_SECTION, true);
    m_textCtrl->StyleSetForeground(wxSTC_PROPS_KEY, wxColour(128, 64, 0));
    m_textCtrl->StyleSetForeground(wxSTC_PROPS_ASSIGNMENT, foreground);
    m_textCtrl->StyleSetForeground(wxSTC_PROPS_DEFVAL, wxColour(160, 0, 0));

    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 40);
    m_textCtrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, foreground);
    m_textCtrl->SetWrapMode(wxSTC_WRAP_NONE);
}

void DialogPresetIni::OnCopyToClipboard(wxCommandEvent& event)
{
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(m_presetIni));
        wxTheClipboard->Close();
    }
}
