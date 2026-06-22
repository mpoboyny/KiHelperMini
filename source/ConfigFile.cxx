#include "prc.hxx"
#include "ConfigFile.hxx"
#include <sstream>

// Public
ConfigFile::ConfigFile()
    : m_xmlDoc(nullptr)
{
    wxXmlParseError err;
    // Suspend logging during this scope
    wxLogNull silence; 
    m_xmlDoc = new wxXmlDocument();
    if (!m_xmlDoc->Load(g_ConfFile, wxXMLDOC_NONE, &err)) {
        if (err.line != 0) {
            TrStr(g_ConfFile);
            // Specific XML syntax error
            m_lastErrorMsg = wxString::Format("XML Error in %s: %s at line %d, col %d", g_ConfFileName, err.message, err.line, err.column);
        }
        else  {
            m_lastErrorMsg = wxString::Format("Error: Could not load: " + g_ConfFile);
        }
        TrStr(m_lastErrorMsg);
    }
}

ConfigFile::~ConfigFile()
{
    delete m_xmlDoc;
}

bool ConfigFile::HaveError(wxString &ErrorMsg) const
{ 
    bool Res = false;
    if (m_xmlDoc == nullptr || !m_lastErrorMsg.empty()) {
        ErrorMsg = m_lastErrorMsg.empty()? "Error: Unknown": m_lastErrorMsg;
        Res = true;
    }
    return Res;
}

ConfigFile::operator wxString() const
{
    wxString Res;
    if (HaveError(Res)) {
        // If there was an error parsing, return the original file contents
        // so the GUI can show the (possibly malformed) file for editing.
        std::ifstream in(g_ConfFile.ToStdString(), std::ios::in | std::ios::binary);
        if (in) {
            std::ostringstream ss;
            ss << in.rdbuf();
            std::string content = ss.str();
            return wxString::FromUTF8(content);
        }
        return "<!-- " + Res + " -->";
    }
    wxStringOutputStream stream;
    m_xmlDoc->Save(stream);
    return stream.GetString();
}
