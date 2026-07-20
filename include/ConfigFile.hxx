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

    ConfigFile();
    ~ConfigFile();

    bool HaveError(wxString &ErrorMsg) const;
    operator wxString() const;

    wxString GetLLamaBinPath();
    ModelList GetModels() const;
    wxString GetCurrentSysName() const;
};

#endif //CONFIG_FILE_HXX
