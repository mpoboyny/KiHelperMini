//
// ScriptHandler.cxx
//

#include "prc.hxx"
#include "ScriptHandler.hxx"

/* static */
wxString ScriptHandler::GetScriptContentWithReplacements(const wxString &scriptPath, const ScriptReplacementsList &replacements)
{
     wxString content;
    if (scriptPath.IsEmpty()) {
        content = wxString::Format("#!/bin/sh\necho Script parameter is empty\n");
    } else {
        wxTextFile file(scriptPath);
        if (!file.Open()) {
            content = wxString::Format("#!/bin/sh\necho Cannot open script file:\n%s", scriptPath);
        } 
        else {
            for (size_t i = 0; i < file.GetLineCount(); ++i) {
                content += file.GetLine(i);
                content += "\n";
            }
            for (const auto& [placeholder, replacement] : replacements) {
                Tr("GetScriptContentWithReplacements: replacing " << placeholder << " with " << replacement);
                content.Replace(placeholder, replacement);
            }
        }
    }
    return content;
}
