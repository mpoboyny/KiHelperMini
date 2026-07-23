//
// ConfigFile.hxx
//

#ifndef CONFIG_FILE_HXX
#define CONFIG_FILE_HXX

class ConfigFile
{
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

    struct ModelInfo
    {
        wxString Path;
        bool Current;
    };
    typedef std::list<ConfigFile::ModelInfo> ModelList;

    struct ChatParameter
    {
        wxString Name;
        wxString Value;
    };
    typedef std::list<ConfigFile::ChatParameter> ChatParameterList;

    ConfigFile();
    ~ConfigFile();

    bool HaveError(wxString &ErrorMsg) const;
    operator wxString() const;

    wxString GetLLamaBinPath() const;
    ModelList GetModels() const;
    wxString GetCurrentSysName() const;
    ChatParameterList GetChatParameters() const;
    wxString GetChatDefPath() const;
};

#endif //CONFIG_FILE_HXX
