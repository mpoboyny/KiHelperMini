//
// WaitDialog.hxx
//

#ifndef WAITDIALOG_HXX
#define WAITDIALOG_HXX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h> 

class WaitDialog {
public:
    // Starts the internal background thread and displays the window immediately
    static void Show(const char* txt);
    
    // Signals the thread to stop, closes the window, and joins the thread cleanly
    static void Hide();

private:
    // Internal loop function running inside the background thread
    static void ThreadLoop(const char* txt);
    
    // Kept private since it's only called internally by the thread
    static void PumpEvents(int frameCounter, const char* baseText); 

    // Static runtime control states
    static std::thread        m_thread;
    static std::atomic<bool>  m_running;

    // Static X11 layout descriptors
    static Display*           m_display;
    static Window             m_window;
    static Pixmap             m_pixmap;
    static GC                 m_gc;
    static int                m_width;
    static int                m_height;
};

#endif // WAITDIALOG_HXX
