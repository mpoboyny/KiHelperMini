//
// DownlodDialog.cxx
//

#include "prc.hxx"
#include "DownlodDialog.hxx"
#include <wx/stattext.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include "../resources/app.xpm"
#include "NetHelper.hxx"


static bool RemoveDirectoryContents(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened()) {
        return false;
    }

    wxString name;
    bool cont = dir.GetFirst(&name, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);
    while (cont) {
        wxString item = path + wxFileName::GetPathSeparator() + name;
        if (wxDirExists(item)) {
            if (!RemoveDirectoryContents(item) || !wxRmdir(item)) {
                return false;
            }
        } else {
            if (!wxRemoveFile(item)) {
                return false;
            }
        }
        cont = dir.GetNext(&name);
    }

    return true;
}

/*static*/
const wxString DownlodDialog::s_defLink = "https://github.com/ggml-org/llama.cpp/archive/refs/heads/master.zip";
/*static*/
const wxSize DownlodDialog::s_defSize = wxSize(600, 350);
/*static*/
const wxString DownlodDialog::s_defSaveDir = "llama.cpp-source/";

DownlodDialog::DownlodDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Download llama.cpp", wxDefaultPosition, s_defSize, wxDEFAULT_DIALOG_STYLE)
{
    TrFu;
    SetIcon(wxIcon(app_xpm));

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fields = new wxFlexGridSizer(2, 2, 10, 10);
    fields->AddGrowableCol(1, 1);

    fields->Add(new wxStaticText(this, wxID_ANY, "From:"), 0, wxALIGN_CENTER_VERTICAL);
    m_fromText = new wxTextCtrl(this, wxID_ANY, DownlodDialog::s_defLink, wxDefaultPosition, wxDefaultSize);
    fields->Add(m_fromText, 1, wxEXPAND);

    fields->Add(new wxStaticText(this, wxID_ANY, "Save in:"), 0, wxALIGN_CENTER_VERTICAL);
    m_saveInText = new wxTextCtrl(this, wxID_ANY, g_ConfDir + s_defSaveDir, wxDefaultPosition, wxDefaultSize);
    fields->Add(m_saveInText, 1, wxEXPAND);

    top->Add(fields, 1, wxALL | wxEXPAND, 12);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    buttonRow->AddStretchSpacer(1);
    buttonRow->Add(new wxButton(this, ID_DONLOAD, "Download"), 0, wxALL, 10);
    buttonRow->Add(new wxButton(this, ID_CANCEL_DOWNLOAD, "Cancel"), 0, wxALL, 10);
    top->Add(buttonRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    SetSizer(top);
    top->SetSizeHints(this);
    SetMinSize(s_defSize);
    SetSize(s_defSize);
    Layout();
    CentreOnParent();

    Bind(wxEVT_BUTTON, &DownlodDialog::OnDonload, this, ID_DONLOAD);
    Bind(wxEVT_BUTTON, &DownlodDialog::OnCancel, this, ID_CANCEL_DOWNLOAD);
}

void DownlodDialog::OnDonload(wxCommandEvent& event)
{
    wxString savePath = m_saveInText->GetValue();
    if (savePath.IsEmpty()) {
        ShowGenericMessageBox("Please specify a save location.", "Download llama.cpp", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!wxDirExists(savePath)) {
        int answer = ShowGenericMessageBox(
            wxString::Format("Folder '%s' does not exist. Create it?", savePath),
            "Download llama.cpp",
            wxYES_NO | wxICON_QUESTION,
            this);
        if (answer != wxID_YES) {
            return;
        }

        if (!wxMkdir(savePath, wxS_DIR_DEFAULT)) {
            ShowGenericMessageBox("Failed to create the save folder.", "Download llama.cpp", wxOK | wxICON_ERROR, this);
            return;
        }
    } else {
        wxDir dir(savePath);
        wxString name;
        bool hasContents = dir.GetFirst(&name);
        
        if (hasContents) {
            int answer = ShowGenericMessageBox(
                wxString::Format("Folder '%s' already exists. Clean it?", savePath),
                "Download llama.cpp",
                wxYES_NO | wxICON_QUESTION,
                this);
            if (answer != wxID_YES) {
                return;
            }

            if (!RemoveDirectoryContents(savePath)) {
                ShowGenericMessageBox("Failed to clean the save folder.", "Download llama.cpp", wxOK | wxICON_ERROR, this);
                return;
            }
        }
    }

    wxString fromUrl = m_fromText->GetValue();
    if (fromUrl.IsEmpty()) {
        ShowGenericMessageBox("Please specify the download source URL.", "Download llama.cpp", wxOK | wxICON_ERROR, this);
        return;
    }

    wxBeginBusyCursor();
    
    if (!NetHelper::IsUrlAvaliable(fromUrl)) {
        wxEndBusyCursor();
        ShowGenericMessageBox("The URL is not available.", "Download llama.cpp", wxOK | wxICON_ERROR, this);
        return;
    }
    
    wxEndBusyCursor();

    int answer = ShowGenericMessageBox(
        wxString::Format("Download from:\n%s\n\nto:\n%s", fromUrl, savePath),
        "Confirm Download",
        wxYES_NO | wxICON_QUESTION,
        this);
    
    if (answer != wxID_YES) {
        return;
    }

    ShowGenericMessageBox("Download started.", "Download llama.cpp", wxOK | wxICON_INFORMATION, this);
}

void DownlodDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}
