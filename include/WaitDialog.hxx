//
// WaitDialog.hxx
//

#ifndef WAITDIALOG_HXX
#define WAITDIALOG_HXX

#if defined(__gnu_linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h> 
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

class WaitDialog {
public:
    // Starts the internal background thread and displays the window immediately
    static void Show(const char* txt);
    
    // Dynamically updates the current status text from the main thread
    static void SetText(const char* txt);
    
    // Signals the thread to stop, closes the window, and joins the thread cleanly
    static void Hide();

private:
    // Internal loop function running inside the background thread
    static void ThreadLoop();
    
    // Kept private since it's only called internally by the thread
    static void PumpEvents(int frameCounter); 

    // Static runtime control states
    static std::thread        m_thread;
    static std::atomic<bool>  m_running;
    
    // Thread-safe storage for the dynamic status text
    static std::string        m_current_text;
    static std::mutex         m_text_mutex;

#if !defined(__gnu_linux__)
    friend void DrawWaitDialogContent(HWND hwnd, HDC hdc);
#endif

#if defined(__gnu_linux__)
    // Static X11 layout descriptors
    static Display*           m_display;
    static Window             m_window;
    static Pixmap             m_pixmap;
    static GC                 m_gc;
    static int                m_width;
    static int                m_height;
#else
    // Static Win32 layout descriptors
    static HWND               m_window;
    static HBITMAP            m_iconBitmap;
    static int                m_width;
    static int                m_height;
#endif
};

#endif // WAITDIALOG_HXX
