//
// DialogCmdRunner.hxx
//

#ifndef DIALOGCMDRUNNER_HXX
#define DIALOGCMDRUNNER_HXX

class DialogCmdRunner : public wxDialog
{
protected:
    DialogCmdRunner(wxWindow* parent,
                    const wxString& dialogTitle,
                    const wxString& headerText,
                    const wxSize& dialogSize);
    ~DialogCmdRunner() override = default;

    void SetupHighlighting();
    virtual void LoadScript() = 0;
    
    wxStyledTextCtrl* m_textCtrl;
};

#endif // DIALOGCMDRUNNER_HXX
