//
// DialogUnzipLlama.cxx
//

#include "prc.hxx"
#include "DialogUnzipLlama.hxx"

DialogUnzipLlama::DialogUnzipLlama(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Unzip llama.cpp", wxDefaultPosition, wxSize(800, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_textCtrl(nullptr)
{
    TrFu;

    SetClientSize(800, 600);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, "extract_llama.sh");
    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    title->SetPosition(wxPoint(10, 10));

    m_textCtrl = new wxStyledTextCtrl(this, wxID_ANY, wxPoint(10, 35), wxSize(780, 515));

    wxButton* closeBtn = new wxButton(this, wxID_OK, "Close");
    closeBtn->SetSize(wxSize(80, 28));
    closeBtn->SetPosition(wxPoint(GetClientSize().GetWidth() - closeBtn->GetSize().GetWidth() - 10,
                                  GetClientSize().GetHeight() - closeBtn->GetSize().GetHeight() - 10));

    CentreOnParent();

    SetupHighlighting();
    LoadScript();
}

void DialogUnzipLlama::SetupHighlighting()
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

void DialogUnzipLlama::LoadScript()
{
    wxString content;
    if (g_ScriptExtraLlamaPath.IsEmpty()) {
        content = "Script not found: scripts/extract_llama.sh";
    } else {
        wxTextFile file(g_ScriptExtraLlamaPath);
        if (!file.Open()) {
            content = wxString::Format("Cannot open script file:\n%s", g_ScriptExtraLlamaPath);
        } else {
            for (size_t i = 0; i < file.GetLineCount(); ++i) {
                content += file.GetLine(i);
                content += "\n";
            }
        }
    }

    m_textCtrl->SetText(content);
    m_textCtrl->SetReadOnly(true);
    m_textCtrl->SetSavePoint();
}
