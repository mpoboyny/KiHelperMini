//
// prc.hxx
//

#ifndef PRC_HXX
#define PRC_HXX

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextbuffer.h>
#include <wx/richtext/richtextxml.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <wx/stc/stc.h> // wxStyledTextCtrl header
#include <wx/dynlib.h>
#include <wx/filename.h>
#include <wx/notebook.h>
#include <wx/artprov.h>
#include <wx/event.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/listctrl.h>
#include <wx/grid.h>
#include <wx/richtooltip.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/accel.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/tokenzr.h>
#include <memory>
#include <utility>
#include <type_traits>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <cctype>
#include <wx/variant.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <list>
#include <vector>

// frameMP
#define _MPTRACE_
#define MP_UNICODE

#include "str.hxx"
#include "trace.hxx"

using namespace frameMP;

extern const int               g_DefaultTxtSize;
extern const wchar_t           *g_DefaultTxtFontName;

extern const wchar_t           *g_APP_NAME;
extern const wxString          g_ConfDir;
extern const wxString          g_ConfFile;

enum MenusIDs
{
    ID_SETTINGS   = wxID_HIGHEST + 50,
    ID_MODEL_INFO = wxID_HIGHEST + 51
};

enum ButtIDs
{
    ID_SHOW_CONF_FOLDER  = wxID_HIGHEST + 1000
};

#endif
