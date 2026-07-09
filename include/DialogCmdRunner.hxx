//
// DialogCmdRunner.hxx
//

#ifndef DIALOGCMDRUNNER_HXX
#define DIALOGCMDRUNNER_HXX

class DialogCmdRunner : public wxDialog
{
    void UseReplacements(wxString& content);

protected:

    wxStyledTextCtrl* m_textCtrl;
    std::map<wxString, wxString> m_scriptReplacements;

    DialogCmdRunner(wxWindow* parent,
                    const wxString& dialogTitle,
                    const wxString& headerText,
                    const wxSize& dialogSize);
    ~DialogCmdRunner() override = default;

    // this functions are called in the constructor
    virtual void  SetupReplacements() = 0;
    void SetupHighlighting();
    wxString GetScriptContentWithReplacements(const wxString &scriptPath);
    virtual void LoadScript() = 0;
    
};

#endif // DIALOGCMDRUNNER_HXX
