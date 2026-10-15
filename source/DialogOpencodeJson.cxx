//
// DialogOpencodeJson.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"
#include "ConfigFile.hxx"
#include "DialogOpencodeJson.hxx"

DialogOpencodeJson::DialogOpencodeJson(wxWindow* parent, const ConfigFile &confFile)
    : DialogCmdRunner(parent, "OpenCode configuration", wxFileName(g_ScriptOpenCodeJsonPath).GetFullName(), wxSize(1000, 900), true, false)
    , m_ConfFile(confFile)
{
    SetupJsonHighlighting();
    LoadScript();
}

void DialogOpencodeJson::SetupJsonHighlighting()
{
    wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    m_textCtrl->SetLexer(wxSTC_LEX_JSON);
    m_textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);

    wxFont monoFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    for (int i = 0; i <= wxSTC_STYLE_MAX; ++i) {
        m_textCtrl->StyleSetFont(i, monoFont);
    }
    m_textCtrl->StyleClearAll();

    m_textCtrl->StyleSetForeground(wxSTC_JSON_PROPERTYNAME, wxColour(0, 0, 180));
    m_textCtrl->StyleSetForeground(wxSTC_JSON_STRING, wxColour(160, 0, 0));
    m_textCtrl->StyleSetForeground(wxSTC_JSON_NUMBER, wxColour(128, 0, 128));
    m_textCtrl->StyleSetForeground(wxSTC_JSON_KEYWORD, wxColour(0, 0, 180));

    m_textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textCtrl->SetMarginWidth(0, 40);
    m_textCtrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, background);
    m_textCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, foreground);
    m_textCtrl->SetWrapMode(wxSTC_WRAP_NONE);
    m_textCtrl->SetTabWidth(4);
}

void DialogOpencodeJson::SetupReplacements(ScriptReplacementsList &replacements)
{
    wxString modelName;
    for (const auto &model : m_ConfFile.GetModels()) {
        if (model.Current) {
            modelName = wxFileName(model.Path).GetName();
            break;
        }
    }

    wxString host = "127.0.0.1";
    wxString port = "8080";
    wxString context = "8192";
    for (const auto &parameter : m_ConfFile.GetServerParameters()) {
        if (parameter.Name == "--host")
            host = parameter.Value;
        else if (parameter.Name == "--port")
            port = parameter.Value;
        else if (parameter.Name == "-c" || parameter.Name == "--ctx-size")
            context = parameter.Value;
    }

    replacements = {
        { "<!-- model -->", modelName },
        { "<!-- host -->", host },
        { "<!-- port -->", port },
        { "<!-- context -->", context },
        { "<!-- limit -->", m_ConfFile.GetOpenCodeDefOutputLimit() }
    };
}

void DialogOpencodeJson::LoadScript()
{
    ScriptReplacementsList replacements;
    SetupReplacements(replacements);
    wxString content = ScriptHandler::GetScriptContentWithReplacements(g_ScriptOpenCodeJsonPath, replacements);
    m_textCtrl->SetReadOnly(false);
    m_textCtrl->SetText(content);
    m_textCtrl->SetSavePoint();
    m_textCtrl->SetReadOnly(true);
}

wxString DialogOpencodeJson::GetJsonContent() const
{
    return m_textCtrl ? m_textCtrl->GetText() : wxString();
}