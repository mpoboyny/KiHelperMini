//
// NetHelper.cxx
//

#include "prc.hxx"
#include "NetHelper.hxx"
#include "../resources/app.xpm"
#include <wx/wfstream.h>

namespace
{
bool HasZipSignature(const wxString& filePath)
{
    wxFile file(filePath);
    if (!file.IsOpened()) {
        return false;
    }

    char zipSignature[4] = {0, 0, 0, 0};
    if (file.Read(zipSignature, sizeof(zipSignature)) != sizeof(zipSignature)) {
        return false;
    }

    return zipSignature[0] == 'P' && zipSignature[1] == 'K';
}

bool CopyStreamToFile(wxInputStream& input, const wxString& targetPath)
{
    wxFileOutputStream output(targetPath);
    if (!output.IsOk()) {
        return false;
    }

    output.Write(input);
    if (!output.IsOk()) {
        output.Close();
        wxRemoveFile(targetPath);
        return false;
    }

    output.Close();
    return HasZipSignature(targetPath);
}

bool DownloadToFile(const wxString& url, const wxString& targetPath)
{
    wxWebRequestSync request = wxWebSessionSync::GetDefault().CreateRequest(url);
    if (request.IsOk()) {
        request.SetMethod("GET");
        request.SetHeader("User-Agent", "KiHelperMiniDir");

        auto result = request.Execute();
        wxWebResponse response = request.GetResponse();

        if (result.state == wxWebRequestSync::State_Completed &&
            response.IsOk() &&
            response.GetStatus() >= 200 && response.GetStatus() < 300) {
            wxInputStream* responseStream = response.GetStream();
            if (responseStream && CopyStreamToFile(*responseStream, targetPath)) {
                return true;
            }
        }
    }

    wxRemoveFile(targetPath);

    wxURL directUrl(url);
    if (directUrl.GetError() != wxURL_NOERR) {
        return false;
    }

    std::unique_ptr<wxInputStream> input(directUrl.GetInputStream());
    if (!input || !input->IsOk()) {
        return false;
    }

    return CopyStreamToFile(*input, targetPath);
}
}

bool NetHelper::IsUrlAvaliable(const wxString& url)
{
    if (url.IsEmpty()) {
        return false;
    }

    wxWebRequestSync request = wxWebSessionSync::GetDefault().CreateRequest(url);
    
    if (!request.IsOk()) {
        return false;
    }

    // Only fetch headers, not the whole file
    request.SetMethod("HEAD");
    
    auto result = request.Execute();
    
    // Check if request completed successfully
    if (result.state != wxWebRequestSync::State_Completed) {
        return false;
    }
    
    wxWebResponse response = request.GetResponse();
    if (!response.IsOk()) {
        return false;
    }
    
    // HTTP 2xx status codes indicate success
    return response.GetStatus() >= 200 && response.GetStatus() < 300;
}

/*static*/
bool NetHelper::Download(wxWindow *caller, const wxString& url, const wxString& to)
{
    TrFu;

    if (url.IsEmpty() || to.IsEmpty()) {
        return false;
    }

    wxDialog* dlg = new wxDialog(caller, wxID_ANY, "", wxDefaultPosition, wxSize(200, 130), wxDEFAULT_DIALOG_STYLE & ~(wxCLOSE_BOX | wxCAPTION));
    dlg->SetIcon(wxIcon(app_xpm));
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticText* text = new wxStaticText(dlg, wxID_ANY, "Be patient...", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    wxGauge* progress = new wxGauge(dlg, wxID_ANY, 100, wxDefaultPosition, wxSize(240, 15), wxGA_HORIZONTAL);
    
    sizer->Add(text, 1, wxALL | wxALIGN_CENTER, 20);
    sizer->Add(progress, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER, 15);

    dlg->SetSizer(sizer);
    dlg->Layout();
    
    dlg->SetTitle("Please wait...");
    dlg->CenterOnParent();
    dlg->Show();

    wxFileName targetInfo;
    targetInfo.AssignDir(to);
    targetInfo.SetFullName("llama.cpp-master.zip");
    
    wxString targetPath = targetInfo.GetFullPath();
    TrStr(targetPath);

    std::atomic<bool> downloadFinished{false};
    bool downloadOk = false;

    std::thread worker([&]() {
        downloadOk = DownloadToFile(url, targetPath);
        downloadFinished = true;
    });

    while (!downloadFinished) {
        progress->Pulse();
        dlg->Update();
        wxYieldIfNeeded();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (worker.joinable()) {
        worker.join();
    }

    dlg->Destroy();
    return downloadOk;
}
