//
// MainDialogPanel.cxx
//

#include "prc.hxx"
#include "MainDialogPanel.hxx"
#include "ConfigFile.hxx"
#include <wx/dir.h>
#include <wx/listbox.h>

CMainDialogPanel::CMainDialogPanel(wxWindow* parent, ConfigFile *cfgFile)
    : wxPanel(parent, wxID_ANY)
    , m_confFile(cfgFile)
    , m_txtLlamaBin(nullptr)
    , m_btnOpenFolder(nullptr)
    , m_listModels(nullptr)
{
    TrFu;

    // Create a static box for grouping config-related controls
    wxStaticBox* groupBox = new wxStaticBox(this, wxID_ANY, "Config settings");
    // Let the static box size to its content (do not force a fixed min height)
    wxStaticBoxSizer* cfgSizer = new wxStaticBoxSizer(groupBox, wxVERTICAL);

    // Row: label + readonly edit + button
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* lbl = new wxStaticText(this, wxID_ANY, "llama bin path: ");
    row->Add(lbl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    wxString sysName = m_confFile ? m_confFile->GetCurrentSysName() : wxString();
    wxString binPath = m_confFile ? m_confFile->GetLLamaBinPath() : wxString();

    // Display only the bin path; do not prefix with system name
    wxString display = binPath;

    m_txtLlamaBin = new wxTextCtrl(this, wxID_ANY, display, wxDefaultPosition, wxSize(400, -1), wxTE_READONLY);
    if (!sysName.IsEmpty())
        m_txtLlamaBin->SetToolTip("System: " + sysName);
    row->Add(m_txtLlamaBin, 1, wxEXPAND | wxRIGHT, 8);

    m_btnOpenFolder = new wxButton(this, wxID_ANY, "Open folder");
    row->Add(m_btnOpenFolder, 0, wxALIGN_CENTER_VERTICAL);

    cfgSizer->Add(row, 0, wxEXPAND | wxALL, 4);

    // Models label and list
    wxStaticText* lblModels = new wxStaticText(this, wxID_ANY, "Models:");
    cfgSizer->Add(lblModels, 0, wxLEFT | wxTOP, 4);

   ConfigFile::ModelList models = m_confFile ? m_confFile->GetModels() : ConfigFile::ModelList();
    if (models.empty()) {
        models.push_back({"No models configured", false});
    }

    // 1. Modus auf wxLC_REPORT ändern und Header mit wxLC_NO_HEADER verstecken
    m_listModels = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 75), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);

    // 2. Eine unsichtbare Spalte hinzufügen, die sich über die gesamte Breite (z.B. 580 Pixel) erstreckt
    m_listModels->InsertColumn(0, "Path", wxLIST_FORMAT_LEFT, 580);

    for (const auto& model : models) {
        long index = m_listModels->GetItemCount();
        
        // 3. Eintrag einfügen
        m_listModels->InsertItem(index, model.Path);
        
        if (model.Current) {
            // Tr(model.Path << " is marked as current model");
            m_listModels->SetItemBackgroundColour(index, g_ColorLightGreen);
        }
    }


    cfgSizer->Add(m_listModels, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

    // Outer sizer with 5px margin around the group
    // Use proportion 0 so the group box sizes to its content and doesn't fill the whole client area
    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
    outerSizer->Add(cfgSizer, 0, wxEXPAND | wxALL, 5);
    SetSizer(outerSizer);
    Layout();

    // Bind events
    if (m_btnOpenFolder)
        m_btnOpenFolder->Bind(wxEVT_BUTTON, &CMainDialogPanel::OnOpenFolder, this);
}

void CMainDialogPanel::OnOpenFolder(wxCommandEvent& WXUNUSED(event))
{
    if (!m_confFile) {
        wxMessageBox("No configuration available", "Info", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxString binPath = m_confFile->GetLLamaBinPath();
    if (binPath.IsEmpty()) {
        wxMessageBox("LLama bin path not configured", "Info", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxFileName fn(binPath);
    wxString folder;
    if (wxDirExists(binPath)) {
        folder = binPath;
    }
    else {
        folder = fn.GetPath();
    }

    if (folder.IsEmpty()) {
        wxMessageBox("Cannot determine folder to open", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    wxString normalized = wxFileName(folder).GetFullPath();

#ifdef _WIN32
    wxString cmd = "explorer \"" + normalized + "\"";
    wxExecute(cmd);
#else
    wxLaunchDefaultApplication(normalized);
#endif
}
