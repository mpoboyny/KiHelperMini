//
// NetHelper.cxx
//

#include "prc.hxx"
#include "NetHelper.hxx"

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
bool NetHelper::Download(const wxString& url, const wxString& to)
{
    TrFu;
    if (url.IsEmpty() || to.IsEmpty()) {
        return false;
    }

    wxWebRequestSync request = wxWebSessionSync::GetDefault().CreateRequest(url);
    if (!request.IsOk()) {
        return false;
    }

    request.SetMethod("GET");
    request.SetStorage(wxWebRequest::Storage_File);

     auto result = request.Execute();
    if (!result) { 
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
    if (tempFile.IsEmpty()) {
        return false;
    }

    wxFileName sourceInfo(tempFile);
    wxString fileName = sourceInfo.GetName() + "." + sourceInfo.GetExt();

    wxFileName targetInfo;
    targetInfo.AssignDir(to);
    targetInfo.SetFullName(fileName);
    
    wxString targetPath = targetInfo.GetFullPath();

    bool res = wxCopyFile(tempFile, targetPath, true);
    wxRemoveFile(tempFile);
    return res; 
}
