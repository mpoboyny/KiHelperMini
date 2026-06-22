//
// ConfigFile.hxx
//

#ifndef CONFIG_FILE_HXX
#define CONFIG_FILE_HXX

class ConfigFile
{
    static const char *s_DefaultConfig;

    wxXmlDocument *m_xmlDoc;
    wxString m_lastErrorMsg;

public:

    struct RoleInfo
    {
        wxString Name;
        bool Current;
    };

    struct RuleInfo
    {
        wxString Name;
        wxString Text;
        bool Current;
    };

    static const wxString s_NoRole;
    static const wxString s_NoRule;


    static bool CheckDefaultConfigFile();

    ConfigFile();
    ~ConfigFile();

    bool HaveError(wxString &ErrorMsg) const;
    operator wxString() const;
    wxString GetLogCheck(wxString &currModelName, wxString &currModelPath) const;
    std::list<RoleInfo> GetRoles() const;
    std::list<RuleInfo> GetRules() const;
};

#endif //CONFIG_FILE_HXX
