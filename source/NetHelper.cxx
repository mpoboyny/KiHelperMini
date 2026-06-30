//
// NetHelper.cxx
//

#include "prc.hxx"
#include "NetHelper.hxx"
#include "../resources/app.xpm"

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

    
    dlg->Update();
    wxYieldIfNeeded();

    std::atomic<bool> downloadFinished{false};
    bool requestOk = false;
    wxWebRequestSync request = wxWebSessionSync::GetDefault().CreateRequest(url);
    
    if (!request.IsOk()) {
        dlg->Destroy();
        return false;
    }

    request.SetMethod("GET");
    request.SetStorage(wxWebRequest::Storage_File);

    auto networkTask = [&request, &downloadFinished, &requestOk]() {
        auto result = request.Execute();
        if (!!result) { 
            requestOk = true;
        }
        downloadFinished = true;
    };

    std::thread workerThread(networkTask);

    while (!downloadFinished) {
        progress->Pulse();
        wxYieldIfNeeded();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    if (workerThread.joinable()) {
        workerThread.join();
    }

    dlg->Destroy();

    if (!requestOk) { 
        return false; 
    }

    wxWebResponse response = request.GetResponse();
    if (!response.IsOk()) {
        return false;
    }

    if (response.GetStatus() < 200 || response.GetStatus() >= 300) {
        return false;
    }
    
    wxString tempFile = response.GetDataFile(); 
    TrStr(tempFile);
    if (tempFile.IsEmpty()) {
        return false;
    }

    wxFileName targetInfo;
    targetInfo.AssignDir(to);
    targetInfo.SetFullName("llama.cpp-master.zip");
    
    wxString targetPath = targetInfo.GetFullPath();
    TrStr(targetPath);
    bool res = false;

    try {
        res = wxCopyFile(tempFile, targetPath, true);
    }
    catch (const std::exception& e) {
        TrStr(e.what());
        return false;
    }
    catch(...) {
        TrStr("Exception occurred while copying temporary file.");
        return false;
    }
    
    try {
        wxRemoveFile(tempFile);
    }
    catch (const std::exception& e) {
        TrStr(wxString("Ignore: ") + e.what());
    }
    catch(...) {
        TrStr(wxString("Ignore: ") + "Exception occurred while removing the temporary file.");
    }

    return res; 
}
