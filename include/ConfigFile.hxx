//
// ConfigFile.hxx
//

#ifndef CONFIG_FILE_HXX
#define CONFIG_FILE_HXX
#include <wx/arrstr.h>

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

    ConfigFile();
    ~ConfigFile();

    bool HaveError(wxString &ErrorMsg) const;
    operator wxString() const;

    wxString GetLLamaBinPath();
    wxArrayString GetModels() const;
    wxString GetCurrentSysName() const;
};

#endif //CONFIG_FILE_HXX
