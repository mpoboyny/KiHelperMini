//
// WaitDialog.cxx
//

#include "prc.hxx"
#include "WaitDialog.hxx"
#include "../resources/app.xpm" // Provides static const char * const app_xpm[]
#include <X11/extensions/Xrandr.h> 

// Initialize static runtime control states
std::thread       WaitDialog::m_thread;
std::atomic<bool> WaitDialog::m_running{false};

// Initialize static X11 layout descriptors
Display*          WaitDialog::m_display = nullptr;
Window            WaitDialog::m_window  = 0;
Pixmap            WaitDialog::m_pixmap  = 0;
GC                WaitDialog::m_gc      = nullptr;
int               WaitDialog::m_width   = 400; // FIXED: Fixed reasonable splash width container
int               WaitDialog::m_height  = 150; // FIXED: Fixed reasonable splash height container

// Keep track of the original embedded icon size
static int icon_width = 0;
static int icon_height = 0;

// Helper function to parse hex colors manually from XPM strings (e.g., "#FF0000")
static unsigned long ParseHexColor(const std::string& colorStr) {
    if (colorStr.empty()) return 0;
    if (colorStr[0] == '#') {
        unsigned long hexVal = 0;
        std::stringstream ss(colorStr.substr(1));
        ss >> std::hex >> hexVal;
        return hexVal;
    }
    return 0; // Black fallback for transparent ("None") or unknown values
}

void WaitDialog::Show(const char* txt) {
    if (m_running) return;
    m_running = true;
    m_thread = std::thread(&WaitDialog::ThreadLoop, txt);
}

void WaitDialog::ThreadLoop(const char* txt) {
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        m_running = false;
        return;
    }

    int screen = DefaultScreen(m_display);
    Window root = RootWindow(m_display, screen);
    Visual* visual = DefaultVisual(m_display, screen);
    int depth = DefaultDepth(m_display, screen);

    // --- MANUAL XPM PARSING ENGINE ---
    int numColors = 0;
    int charsPerPixel = 0;
    
    // FIXED: Correct array indexing for C++17 stringstream
    std::stringstream headerStream(app_xpm[0]);
    headerStream >> icon_width >> icon_height >> numColors >> charsPerPixel;

    std::map<std::string, unsigned long> colorMap;
    for (int i = 0; i < numColors; ++i) {
        std::string line = app_xpm[1 + i];
        std::string key = line.substr(0, charsPerPixel);
        size_t cPos = line.find(" c ");
        if (cPos != std::string::npos) {
            std::string colorVal = line.substr(cPos + 3);
            colorMap[key] = ParseHexColor(colorVal);
        }
    }

    int bytesPerPixel = 4;
    std::vector<char> rawBuffer(icon_width * icon_height * bytesPerPixel, 0);

    int pixelDataOffset = 1 + numColors;
    for (int y = 0; y < icon_height; ++y) {
        std::string rowStr = app_xpm[pixelDataOffset + y];
        for (int x = 0; x < icon_width; ++x) {
            std::string pixelKey = rowStr.substr(x * charsPerPixel, charsPerPixel);
            unsigned long color = colorMap[pixelKey];

            int destIdx = (y * icon_width + x) * bytesPerPixel;
            rawBuffer[destIdx + 0] = (color & 0x0000FF);        // Blue
            rawBuffer[destIdx + 1] = (color & 0x00FF00) >> 8;   // Green
            rawBuffer[destIdx + 2] = (color & 0xFF0000) >> 16;  // Red
            rawBuffer[destIdx + 3] = 0;                         // Padding Alpha
        }
    }

    // --- AUTOMATIC ACTIVE MONITOR DETECTION VIA FOCUS WINDOW (PURE X11) ---
    m_width = 400;  
    m_height = 150; 

    // Default fallbacks: entire virtual desktop area
    XWindowAttributes root_attrs;
    XGetWindowAttributes(m_display, root, &root_attrs);
    
    int monitor_x = 0;
    int monitor_y = 0;
    int monitor_w = root_attrs.width;
    int monitor_h = root_attrs.height;

    // Get current mouse cursor position as an auxiliary metric
    Window root_return, child_return;
    int mouse_x = 0, mouse_y = 0;
    int win_x_ret = 0, win_y_ret = 0;
    unsigned int mask_return = 0;
    XQueryPointer(m_display, root, &root_return, &child_return, &mouse_x, &mouse_y, &win_x_ret, &win_y_ret, &mask_return);

    // Dynamic split fallback for side-by-side dual monitors of identical resolution
    if (root_attrs.width == 3840 && root_attrs.height <= 1200) {
        monitor_w = 1920;
        monitor_x = (mouse_x >= 1920) ? 1920 : 0;
    } else if (root_attrs.width == 5120 && root_attrs.height <= 1600) {
        monitor_w = 2560;
        monitor_x = (mouse_x >= 2560) ? 2560 : 0;
    } else {
        // Advanced heuristic: inspect the window that currently has the focus (e.g., your IDE or terminal)
        Window focused_window = 0;
        int revert_to_return = 0;
        XGetInputFocus(m_display, &focused_window, &revert_to_return);

        if (focused_window != 0 && focused_window != PointerRoot && focused_window != None) {
            XWindowAttributes focused_attrs;
            if (XGetWindowAttributes(m_display, focused_window, &focused_attrs) != 0) {
                int absolute_root_x = 0, absolute_root_y = 0;
                Window child_translated = 0;
                XTranslateCoordinates(m_display, focused_window, root, 0, 0, &absolute_root_x, &absolute_root_y, &child_translated);

                // If the terminal/IDE is contained on a sub-screen, extract boundaries dynamically
                if (focused_attrs.width >= 640 && focused_attrs.width < root_attrs.width) {
                    monitor_w = (absolute_root_x >= root_attrs.width / 2 || focused_attrs.width > 1920) ? (root_attrs.width - (root_attrs.width / 2)) : (root_attrs.width / 2);
                    monitor_x = (absolute_root_x >= root_attrs.width / 2) ? (root_attrs.width / 2) : 0;
                }
            }
        }
    }

    // Precise mathematical centering calculation based on the dynamic monitor profile
    int win_x = monitor_x + ((monitor_w - m_width) / 2);
    int win_y = monitor_y + ((monitor_h - m_height) / 2);

    // Enforce window constraints using absolute redirect to bypass window manager panels
    XSetWindowAttributes window_attributes;
    window_attributes.override_redirect = True; 
    window_attributes.background_pixel = BlackPixel(m_display, screen);

    m_window = XCreateWindow(
        m_display, root, win_x, win_y, m_width, m_height, 0,
        depth, InputOutput, visual,
        CWOverrideRedirect | CWBackPixel, &window_attributes
    );

    // Apply native splash settings properties onto the window instance
    Atom window_type = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE", False);
    Atom type_splash = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    XChangeProperty(m_display, m_window, window_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&type_splash), 1);

    XMapWindow(m_display, m_window);
    XFlush(m_display);

    m_gc = XCreateGC(m_display, m_window, 0, nullptr);
    XSetForeground(m_display, m_gc, WhitePixel(m_display, screen));

    // Convert raw unpacked icon bits into GPU server memory container
    m_pixmap = XCreatePixmap(m_display, m_window, icon_width, icon_height, depth);
    XImage* xImage = XCreateImage(
        m_display, visual, depth, ZPixmap, 0,
        rawBuffer.data(), icon_width, icon_height, 32, 0
    );

    XPutImage(m_display, m_pixmap, m_gc, xImage, 0, 0, 0, 0, icon_width, icon_height);
    xImage->data = nullptr; 
    XDestroyImage(xImage);

    XFlush(m_display);
    int frameCounter = 0;

    // Continuous autonomous window rendering dispatch loop context
    while (m_running) {
        PumpEvents(frameCounter, txt);
        frameCounter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Resource deallocation teardown
    if (m_gc) XFreeGC(m_display, m_gc);
    if (m_pixmap) XFreePixmap(m_display, m_pixmap);
    if (m_window) XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
    
    m_display = nullptr;
    m_window = 0;
    m_pixmap = 0;
    m_gc = nullptr;
}

void WaitDialog::PumpEvents(int frameCounter, const char* baseText) {
    if (!m_display || !m_window || !m_pixmap) return;

    XEvent event;
    while (XPending(m_display)) {
        XNextEvent(m_display, &event);
    }

    // Clear whole container canvas before drawing frame layouts
    XClearWindow(m_display, m_window);

    // FIXED: Calculate centering coordinates to place the small icon at the top center
    int icon_x = (m_width - icon_width) / 2;
    int icon_y = 25; // Padding from top edge
    XCopyArea(m_display, m_pixmap, m_window, m_gc, 0, 0, icon_width, icon_height, icon_x, icon_y);

    // String formatting sequence
    char animatedText[256];
    std::memset(animatedText, 0, sizeof(animatedText));
    std::strncpy(animatedText, baseText, sizeof(animatedText) - 5);
    animatedText[sizeof(animatedText) - 5] = '\0';
    
    int dots = (frameCounter % 4);
    for (int i = 0; i < dots; ++i) {
        std::strcat(animatedText, ".");
    }

    // FIXED: Position text lower, at the bottom section of the scaled window shell
    int text_y = m_height - 30;
    
    // Estimate text width roughly to center it inside the 400px container width
    int approximate_text_width = std::strlen(animatedText) * 6; 
    int text_x = (m_width - approximate_text_width) / 2;
    if (text_x < 10) text_x = 10; // Left-edge bound safety guard

    XDrawString(m_display, m_window, m_gc, text_x, text_y, animatedText, std::strlen(animatedText));
    
    XFlush(m_display);
}

void WaitDialog::Hide() {
    if (!m_running) return;
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
