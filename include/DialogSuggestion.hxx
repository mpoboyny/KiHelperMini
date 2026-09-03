//
// DialogSuggestion.hxx
//

#ifndef DIALOGSUGGESTION_HXX
#define DIALOGSUGGESTION_HXX

class ConfigFile;

class DialogSuggestion : public wxDialog
{
    static const wxSize s_defSize;

    wxTextCtrl* m_modelText;

public:
    DialogSuggestion(wxWindow* parent, const ConfigFile& confFile);
    ~DialogSuggestion() override = default;
};

#endif // DIALOGSUGGESTION_HXX
