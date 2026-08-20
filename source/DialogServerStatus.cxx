//
// DialogServerStatus.cxx
//

#include "prc.hxx"
#include "DialogServerStatus.hxx"
#include "WaitDialog.hxx"
#include "../resources/app.xpm"
#include <filesystem>
#include <fstream>
#if defined(__gnu_linux__)
#include <signal.h>
#endif

/*static*/
const wxSize DialogServerStatus::s_defSize = wxSize(700, 350);

DialogServerStatus::DialogServerStatus(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Server status", wxDefaultPosition, s_defSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    TrFu;
    SetIcon(wxIcon(app_xpm));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_textCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    mainSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer* buttonRowSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonRowSizer->AddStretchSpacer(1);

    m_killBtn = new wxButton(this, ID_KILL_SERVER, "Kill", wxDefaultPosition, wxSize(-1, FromDIP(28)));
    m_killBtn->Enable(false);
    m_closeBtn = new wxButton(this, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(-1, FromDIP(28)));

    buttonRowSizer->Add(m_killBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    buttonRowSizer->Add(m_closeBtn, 0, wxALIGN_CENTER_VERTICAL);

    mainSizer->Add(buttonRowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);
    SetMinSize(s_defSize);
    SetSize(s_defSize);
    Layout();
    CentreOnParent();

    Bind(wxEVT_BUTTON, &DialogServerStatus::OnKill, this, ID_KILL_SERVER);
    Bind(wxEVT_BUTTON, &DialogServerStatus::OnClose, this, wxID_CANCEL);

    UpdateStatus();
}

long DialogServerStatus::FindServerPid()
{
#if defined(__gnu_linux__)
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator("/proc", ec)) {
        if (!entry.is_directory(ec)) continue;
        std::string name = entry.path().filename().string();
        if (name.empty() || !std::all_of(name.begin(), name.end(), ::isdigit)) continue;

        std::filesystem::path commPath = entry.path() / "comm";
        std::ifstream commFile(commPath);
        if (commFile.is_open()) {
            std::string comm;
            std::getline(commFile, comm);
            if (comm == "llama-server") {
                return std::stol(name);
            }
        }

        std::filesystem::path exePath = entry.path() / "exe";
        std::filesystem::path target = std::filesystem::read_symlink(exePath, ec);
        if (!ec && target.filename() == "llama-server") {
            return std::stol(name);
        }
    }
#endif
    return -1;
}

wxArrayString DialogServerStatus::GetLastLogLines(size_t maxLines)
{
    wxArrayString lines;
    if (!wxFileExists(g_SrvLogFile)) {
        return lines;
    }

    wxTextFile file(g_SrvLogFile);
    if (!file.Open()) {
        return lines;
    }

    size_t total = file.GetLineCount();
    size_t start = (total > maxLines) ? total - maxLines : 0;
    for (size_t i = start; i < total; ++i) {
        lines.Add(file.GetLine(i));
    }
    file.Close();
    return lines;
}

void DialogServerStatus::UpdateStatus()
{
    m_serverPid = FindServerPid();

    wxString status;
    status += wxString::Format("Server log file: %s\n", g_SrvLogFile);

    if (m_serverPid > 0) {
        status += wxString::Format("Server is running (PID: %ld)\n\n", m_serverPid);
        m_killBtn->Enable(true);
    } else {
        status += "Server is not running.\n\n";
        m_killBtn->Enable(false);
    }

    status += "Last 10 lines from log:\n";
    wxArrayString lastLines = GetLastLogLines(10);
    if (lastLines.IsEmpty()) {
        status += "(No log entries available)\n";
    } else {
        for (const auto& line : lastLines) {
            status += line + "\n";
        }
    }

    m_textCtrl->SetValue(status);
}

void DialogServerStatus::OnKill(wxCommandEvent& event)
{
    TrFu;
    if (m_serverPid > 0) {
#if defined(__gnu_linux__)
        ::kill(m_serverPid, SIGTERM);
#endif
        WaitDialog::Show("Terminate server");
        wxMilliSleep(2000);
        WaitDialog::Hide();
        UpdateStatus();
        m_textCtrl->SetValue("If server still running, kill it later\n\n\n" + m_textCtrl->GetValue());
        m_killBtn->Enable(false);
    }
}

void DialogServerStatus::OnClose(wxCommandEvent& event)
{
    TrFu;
    EndModal(wxID_CANCEL);
}
