//
// NetHelper.cxx
//

#include "prc.hxx"
#include "NetHelper.hxx"
#include <wx/webrequest.h>

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

