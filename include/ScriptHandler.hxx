//
// ScriptHandler.hxx
//

#ifndef SCRIPTHANDLER_HXX
#define SCRIPTHANDLER_HXX

class ScriptHandler
{
    public:
        static wxString GetScriptContentWithReplacements(const wxString &scriptPath, const ScriptReplacementsList &replacements);
};

#endif // SCRIPTHANDLER_HXX
