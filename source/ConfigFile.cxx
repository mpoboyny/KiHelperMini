#include "prc.hxx"
#include "ConfigFile.hxx"
#include <sstream>

namespace
{
    wxXmlNode* GetSystemsNode(wxXmlNode* root)
    {
        if (!root)
            return nullptr;

        if (root->GetName().CmpNoCase("Systems") == 0)
            return root;

        for (wxXmlNode* child = root->GetChildren(); child; child = child->GetNext()) {
            if (child->GetName().CmpNoCase("Systems") == 0)
                return child;
        }

        return nullptr;
    }

    wxXmlNode* GetSystemNode(wxXmlNode* systemsNode, const wxString& sysName)
    {
        if (!systemsNode || sysName.IsEmpty())
            return nullptr;

        for (wxXmlNode* system = systemsNode->GetChildren(); system; system = system->GetNext()) {
            if (system->GetType() != wxXML_ELEMENT_NODE)
                continue;
            if (system->GetName().CmpNoCase("System") != 0)
                continue;

            wxString nameAttr = system->GetAttribute("Name", "");
            if (nameAttr.IsEmpty())
                nameAttr = system->GetAttribute("name", "");

            if (nameAttr.CmpNoCase(sysName) == 0)
                return system;
        }

        return nullptr;
    }

    wxXmlNode* GetChildNode(wxXmlNode* parent, const wxString& name)
    {
        if (!parent)
            return nullptr;

        for (wxXmlNode* child = parent->GetChildren(); child; child = child->GetNext()) {
            if (child->GetType() != wxXML_ELEMENT_NODE)
                continue;
            if (child->GetName().CmpNoCase(name) == 0)
                return child;
        }

        return nullptr;
    }

    ConfigFile::ChatParameterList GetParametersFromContainer(wxXmlNode* paramsContainerNode)
    {
        ConfigFile::ChatParameterList res;
        if (!paramsContainerNode)
            return res;

        for (wxXmlNode* param = paramsContainerNode->GetChildren(); param; param = param->GetNext()) {
            if (param->GetType() != wxXML_ELEMENT_NODE)
                continue;
            if (param->GetName().CmpNoCase("Parameter") != 0)
                continue;

            wxString name = param->GetAttribute("Name", "");
            if (name.IsEmpty())
                name = param->GetAttribute("name", "");

            wxString value = param->GetAttribute("Value", "");
            if (value.IsEmpty())
                value = param->GetAttribute("value", "");

            if (!name.IsEmpty())
                res.push_back({name, value});
        }

        return res;
    }

    wxString GetDefPathFromContainer(wxXmlNode* paramsContainerNode, const wxString& fallback)
    {
        wxString res = fallback;
        if (!paramsContainerNode)
            return res;

        for (wxXmlNode* defBin = paramsContainerNode->GetChildren(); defBin; defBin = defBin->GetNext()) {
            if (defBin->GetType() != wxXML_ELEMENT_NODE)
                continue;
            if (defBin->GetName().CmpNoCase("DefBin") != 0)
                continue;

            wxString path = defBin->GetAttribute("Path", "");
            if (path.IsEmpty())
                path = defBin->GetAttribute("path", "");

            if (!path.IsEmpty()) {
                res = path;
                break;
            }
        }

        return res;
    }
}

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

// Return current system name matching config.xml <System Name="..."> values
wxString ConfigFile::GetCurrentSysName() const
{
#ifdef _WIN32
    return wxString("Windows");
#elif defined(__APPLE__)
    return wxString("Mac");
#elif defined(__linux__)
    return wxString("Linux");
#else
    return wxString();
#endif
}

ConfigFile::ModelList ConfigFile::GetModels() const
{
    ConfigFile::ModelList res;
    wxString err;
    if (HaveError(err))
        return res;
    if (m_xmlDoc == nullptr)
        return res;

    wxString sysName = GetCurrentSysName();

    wxXmlNode* root = m_xmlDoc->GetRoot(); // <Settings>
    if (!root)
        return res;

    // Find <Systems>
    wxXmlNode* systemsNode = nullptr;
    for (wxXmlNode* child = root->GetChildren(); child; child = child->GetNext()) {
        if (child->GetName().CmpNoCase("Systems") == 0) {
            systemsNode = child;
            break;
        }
    }
    if (!systemsNode)
        return res;

    // Iterate <System Name="..."> nodes
    for (wxXmlNode* system = systemsNode->GetChildren(); system; system = system->GetNext()) {
        if (system->GetName().CmpNoCase("System") != 0)
            continue;
        wxString nameAttr = system->GetAttribute("Name", "");
        if (nameAttr.IsEmpty())
            continue;
        if (!sysName.IsEmpty() && nameAttr.CmpNoCase(sysName) != 0)
            continue;

        // Found matching System entry, look for Models -> Model
        for (wxXmlNode* models = system->GetChildren(); models; models = models->GetNext()) {
            if (models->GetName().CmpNoCase("Models") != 0)
                continue;
            for (wxXmlNode* model = models->GetChildren(); model; model = model->GetNext()) {
                if (model->GetName().CmpNoCase("Model") != 0)
                    continue;
                wxString fileAttr = model->GetAttribute("File", "");
                if (fileAttr.IsEmpty())
                    continue;

                wxFileName fn(fileAttr);
                if (!fn.IsAbsolute()) {
                    wxFileName cfg(g_ConfFile);
                    wxFileName full(cfg.GetPath(), fileAttr);
                    fn = full;
                }

                res.push_back({fn.GetFullPath(), model->GetAttribute("Current", "false").CmpNoCase("true") == 0});
            }
            // return after processing Models for matched system
            return res;
        }
    }

    return res;
}

wxString ConfigFile::GetLLamaBinPath() const
{
    wxString Res;
    if (HaveError(Res))
        return m_lastErrorMsg;
    if (m_xmlDoc == nullptr)
        return wxString();

    // Decide which <System Name="..."> to select based on current platform
    wxString sysName = GetCurrentSysName();

    wxXmlNode* root = m_xmlDoc->GetRoot(); // <Settings>
    if (!root)
        return wxString();

    // Find <Systems>
    wxXmlNode* systemsNode = nullptr;
    for (wxXmlNode* child = root->GetChildren(); child; child = child->GetNext()) {
        if (child->GetName().CmpNoCase("Systems") == 0) {
            systemsNode = child;
            break;
        }
    }
    if (!systemsNode)
        return wxString();

    // Iterate <System Name="..."> nodes
    for (wxXmlNode* system = systemsNode->GetChildren(); system; system = system->GetNext()) {
        if (system->GetName().CmpNoCase("System") != 0)
            continue;
        wxString nameAttr = system->GetAttribute("Name", "");
        if (nameAttr.IsEmpty())
            continue;
        if (!sysName.IsEmpty() && nameAttr.CmpNoCase(sysName) != 0)
            continue;

        // Found matching System entry, look for LLamaCpp -> Bin
        for (wxXmlNode* ll = system->GetChildren(); ll; ll = ll->GetNext()) {
            if (ll->GetName().CmpNoCase("LLamaCpp") != 0)
                continue;
            for (wxXmlNode* bin = ll->GetChildren(); bin; bin = bin->GetNext()) {
                if (bin->GetName().CmpNoCase("Bin") != 0)
                    continue;
                wxString pathAttr = bin->GetAttribute("Path", "");
                if (pathAttr.IsEmpty())
                    continue;

                wxFileName fn(pathAttr);
                if (!fn.IsAbsolute()) {
                    // If relative, resolve against config file directory
                    wxFileName cfg(g_ConfFile);
                    wxString cfgDir = cfg.GetPath();
                    wxFileName full(cfgDir, pathAttr);
                    fn = full;
                }

                // Return normalized full path (no trailing separator)
                return fn.GetFullPath();
            }
        }
    }

    // Not found
    return wxString();
}

ConfigFile::ChatParameterList ConfigFile::GetChatParameters() const
{
    TrFu;
    wxString sysName = GetCurrentSysName();
    TrStr(sysName);

    ChatParameterList res;
    wxString err;
    
    if (HaveError(err)) 
    {
        TrStr(err);
        return res;
    }
    
    TrRet(m_xmlDoc == nullptr, res);
    TrRet(sysName.IsEmpty(), res);
    
    wxXmlNode* root = m_xmlDoc->GetRoot();
    TrRet(!root, res);

    wxXmlNode* systemsNode = GetSystemsNode(root);
    TrRet(!systemsNode, res);

    wxXmlNode* targetSystemNode = GetSystemNode(systemsNode, sysName);
    
    TrRet(!targetSystemNode, res);
    
    wxXmlNode* paramsContainerNode = GetChildNode(targetSystemNode, "LlamaChatParameters");
    TrRet(!paramsContainerNode, res);

    return GetParametersFromContainer(paramsContainerNode);
}

wxString ConfigFile::GetChatDefPath() const
{
    TrFu;
    wxString sysName = GetCurrentSysName();
    TrStr(sysName);
    wxString res = GetLLamaBinPath();
    wxString err;
     if (HaveError(err)) 
    {
        TrStr(err);
        return res;
    }
    
    TrRet(m_xmlDoc == nullptr, res);
    TrRet(sysName.IsEmpty(), res);
    
    wxXmlNode* root = m_xmlDoc->GetRoot();
    TrRet(!root, res);
    wxXmlNode* systemsNode = GetSystemsNode(root);
    TrRet(!systemsNode, res);

    wxXmlNode* targetSystemNode = GetSystemNode(systemsNode, sysName);
    
    TrRet(!targetSystemNode, res);
    
    wxXmlNode* paramsContainerNode = GetChildNode(targetSystemNode, "LlamaChatParameters");
    TrRet(!paramsContainerNode, res);

    res = GetDefPathFromContainer(paramsContainerNode, res);
    TrStr(res);
    return res;
}

ConfigFile::ChatParameterList ConfigFile::GetServerParameters() const
{
    TrFu;
    wxString sysName = GetCurrentSysName();
    TrStr(sysName);

    ChatParameterList res;
    wxString err;

    if (HaveError(err))
    {
        TrStr(err);
        return res;
    }

    TrRet(m_xmlDoc == nullptr, res);
    TrRet(sysName.IsEmpty(), res);

    wxXmlNode* root = m_xmlDoc->GetRoot();
    TrRet(!root, res);

    wxXmlNode* systemsNode = GetSystemsNode(root);
    TrRet(!systemsNode, res);

    wxXmlNode* targetSystemNode = GetSystemNode(systemsNode, sysName);
    TrRet(!targetSystemNode, res);

    wxXmlNode* paramsContainerNode = GetChildNode(targetSystemNode, "LlamaServerParameters");
    if (!paramsContainerNode)
        return GetChatParameters();

    return GetParametersFromContainer(paramsContainerNode);
}

wxString ConfigFile::GetServerDefPath() const
{
    TrFu;
    wxString sysName = GetCurrentSysName();
    TrStr(sysName);
    wxString res = GetLLamaBinPath();
    wxString err;
    if (HaveError(err))
    {
        TrStr(err);
        return res;
    }

    TrRet(m_xmlDoc == nullptr, res);
    TrRet(sysName.IsEmpty(), res);

    wxXmlNode* root = m_xmlDoc->GetRoot();
    TrRet(!root, res);

    wxXmlNode* systemsNode = GetSystemsNode(root);
    TrRet(!systemsNode, res);

    wxXmlNode* targetSystemNode = GetSystemNode(systemsNode, sysName);
    TrRet(!targetSystemNode, res);

    wxXmlNode* paramsContainerNode = GetChildNode(targetSystemNode, "LlamaServerParameters");
    if (!paramsContainerNode)
        return GetChatDefPath();

    res = GetDefPathFromContainer(paramsContainerNode, res);
    TrStr(res);
    return res;
}
