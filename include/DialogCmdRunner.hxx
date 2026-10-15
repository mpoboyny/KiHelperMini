//
// DialogCmdRunner.hxx
//

#ifndef DIALOGCMDRUNNER_HXX
#define DIALOGCMDRUNNER_HXX

class DialogCmdRunner : public wxDialog
{
    wxString m_ScriptFileName;
    wxString m_ScriptFileExtension;

    bool m_RunScriptSync;

protected:

    wxStyledTextCtrl* m_textCtrl;
    wxBoxSizer* m_buttonRow;
    
    DialogCmdRunner(wxWindow* parent,
                    const wxString& dialogTitle,
                    const wxString& headerText,
                    const wxSize& dialogSize,
                    bool runScriptSync = true,
                    bool showDoItButton = true);
    ~DialogCmdRunner() override = default;

    void SetupHighlighting();
    virtual void LoadScript() = 0;
    void AddCustomButton(wxButton* button);
    
    void OnDialogDoIt(wxCommandEvent& event);
};

#endif // DIALOGCMDRUNNER_HXX
