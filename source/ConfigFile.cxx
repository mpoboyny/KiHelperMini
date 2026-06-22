#include "prc.hxx"
#include "ConfigFile.hxx"

/* static */
const char *ConfigFile::s_DefaultConfig = "<Settings>\n"
                                          "\t<LocalModels Folder=\"/home/tato/LocalKiModels/\" >\n"
                                            "\t\t<Model Name=\"DeepSeek-R1-Distill-Qwen-1.5B.Q5_K_M\" File=\"DeepSeek-R1-Distill-Qwen-1.5B.Q5_K_M.gguf\" />\n"
                                            "\t\t<Model Name=\"Mistral-7B-Instruct-v0.3-Q5_K_M\" File=\"Mistral-7B-Instruct-v0.3-Q5_K_M.gguf\"/>\n"
                                            "\t\t<Model Name=\"Qwen2.5-14B-Instruct-Q4_K_M\" File= \"Qwen2.5-14B-Instruct-Q4_K_M.gguf\" />\n"
                                          "\t</LocalModels>\n"
                                          "\t<CurrentModel Name=\"DeepSeek-R1-Distill-Qwen-1.5B.Q5_K_M\" />\n"
                                          "\t<Roles>\n"
                                          "\t\t<Role Name=\"Model tester\" Current=\"true\"/>\n"
                                          "\t\t<Role Name=\"Code Optimizer\" />\n"
                                          "\t\t<Role Name=\"Bug Hunter\" />\n"
                                          "\t\t<Role Name=\"C/C++/C# Expert\" />\n"
                                          "\t</Roles>\n"
                                          "\t<Rules>\n"
                                          "\t\t<Rule Name=\"Default\">\n"
                                          "\t\t\t<Text>Minimal output</Text>\n"
                                          "\t\t</Rule>\n"
                                          "\t\t<Rule Name=\"No suggestions\" Current=\"true\">\n"
                                          "\t\t\t<Text>Only necessary suggestions</Text>\n"
                                          "\t\t</Rule>\n"
                                          "\t\t</Rules>\n"
                                          "</Settings>\n";
/*static*/
const wxString ConfigFile::s_NoRole = "--- no ---";
/*static*/
const wxString ConfigFile::s_NoRule = "--- no ---";

// static
bool ConfigFile::CheckDefaultConfigFile()
{
    TrFu;
    TrStr(g_ConfFile);
    if (!wxFileName::DirExists(g_ConfDir))
    {
        // wxPATH_MKDIR_FULL ensures all parent directories are created
        wxFileName::Mkdir(g_ConfDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    bool res = wxFileName::FileExists(g_ConfFile);
    if (!res){
        wxFile file;
        res = file.Create(g_ConfFile);
        if (res) {
            // Write the initial default configuration content
            file.Write(s_DefaultConfig);
            // Close the file handle immediately after creation
            file.Close();
        }
    }
    return res;
}

// Private

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
            // Specific XML syntax error
            m_lastErrorMsg = wxString::Format("XML Error: %s at line %d, col %d", err.message, err.line, err.column);
        }
        else  {
            m_lastErrorMsg = wxString::Format("Error: Could not load: " + g_ConfFile);
        }
        TrStr(m_lastErrorMsg);
        
        // Load default config as fallback so m_xmlDoc is fully initialized
        wxStringInputStream stream(s_DefaultConfig);
        m_xmlDoc->Load(stream);
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
        return "<!-- " + Res + " -->";
    }
    wxStringOutputStream stream;
    m_xmlDoc->Save(stream);
    return stream.GetString();
}

wxString ConfigFile::GetLogCheck(wxString &currModelName, wxString &currModelPath) const
{
    wxString Res;
    if (HaveError(Res)) {
        return Res;
    }
    // Root
    wxXmlNode* root = m_xmlDoc->GetRoot();
    if (!root || root->GetName() != "Settings") {
        Res = "Error: root 'Settings' not found";
        return Res;
    }
    // LocalModels Folder
    wxXmlNode *localModelsNode = nullptr;
    wxXmlNode *currentModelNode = nullptr;
    wxString modelsFolder;
    for (wxXmlNode* node = root->GetChildren(); node; node = node->GetNext()) {
        if (!localModelsNode && node->GetName() == "LocalModels") {
            modelsFolder = node->GetAttribute("Folder", "");
            if (!wxFileName::DirExists(modelsFolder)) {
                Res = "Error: <LocalModels Folder... /> not exists";
                return Res;
            }
            else {
                localModelsNode = node;
                Res = "Info: Folder of local models: " + modelsFolder;
            }
        }
        if (!currentModelNode && node->GetName() == "CurrentModel") {
            currentModelNode = node;
        }
    }
    if (!currentModelNode) {
        Res += "\nError: CurrentModel not defined";
        return Res;
    }
    currModelName = currentModelNode->GetAttribute("Name", "");
    Res += "\nInfo: CurrentModel Name: " + currModelName;
    wxString currModelFileName;
    for (wxXmlNode* node = localModelsNode->GetChildren(); node; node = node->GetNext()) {
        if (node->GetName() == "Model") {
            if (node->GetAttribute("Name", "") == currModelName) {
                currModelFileName = node->GetAttribute("File", "");
                Res += "\nInfo: CurrentModel File: " + currModelFileName;
                break;
            }
        }
    }
    
    wxFileName fn;
    fn.AssignDir(modelsFolder);
    fn.SetFullName(currModelFileName);
    currModelPath = fn.GetFullPath();

    if (wxFileName::FileExists(currModelPath))
        Res += "\nInfo: CurrentModel file exists: " + currModelPath;
    else
        Res += "\nError: CurrentModel file not exists: " + currModelPath;
    
    return Res;
}

std::list<ConfigFile::RoleInfo> ConfigFile::GetRoles() const
{
    std::list<RoleInfo> resultList;
    
    if (m_xmlDoc) {
        wxXmlNode* root = m_xmlDoc->GetRoot();
        if (root && root->GetName() == "Settings") {
            for (wxXmlNode* node = root->GetChildren(); node; node = node->GetNext()) {
                if (node->GetName() == "Roles") {
                    for (wxXmlNode* roleNode = node->GetChildren(); roleNode; roleNode = roleNode->GetNext()) {
                        if (roleNode->GetName() == "Role") {
                            wxString roleName = roleNode->GetAttribute("Name", "");
                            if (!roleName.IsEmpty()) {
                                bool isCurrent = (roleNode->GetAttribute("Current", "false").Lower() == "true");
                                resultList.push_back({roleName, isCurrent});
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    // Ensure s_NoRole is at the front, and remove duplicates of it
    resultList.remove_if([](const RoleInfo& info) { return info.Name == s_NoRole; });
    resultList.push_front({s_NoRole, false});
    
    return resultList;
}

std::list<ConfigFile::RuleInfo> ConfigFile::GetRules() const
{
    std::list<RuleInfo> resultList;
    
    if (m_xmlDoc) {
        wxXmlNode* root = m_xmlDoc->GetRoot();
        if (root && root->GetName() == "Settings") {
            for (wxXmlNode* node = root->GetChildren(); node; node = node->GetNext()) {
                if (node->GetName() == "Rules") {
                    for (wxXmlNode* ruleNode = node->GetChildren(); ruleNode; ruleNode = ruleNode->GetNext()) {
                        if (ruleNode->GetName() == "Rule") {
                            wxString ruleName = ruleNode->GetAttribute("Name", "");
                            if (!ruleName.IsEmpty()) {
                                bool isCurrent = (ruleNode->GetAttribute("Current", "false").Lower() == "true");
                                
                                wxString ruleText;
                                for (wxXmlNode* child = ruleNode->GetChildren(); child; child = child->GetNext()) {
                                    if (child->GetName() == "Text") {
                                        ruleText = child->GetNodeContent().Trim(false).Trim(true);
                                        break;
                                    }
                                }

                                resultList.push_back({ruleName, ruleText, isCurrent});
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    resultList.remove_if([](const RuleInfo& info) { return info.Name == s_NoRule; });
    resultList.sort([](const RuleInfo& a, const RuleInfo& b) { return a.Name.Cmp(b.Name) < 0; });
    resultList.push_front({s_NoRule, "", false});

    return resultList;
}
