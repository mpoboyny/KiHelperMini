//
// WaitDialog.cxx
//

#include "prc.hxx"
#include "WaitDialog.hxx"
#include "../resources/app.xpm" // Provides static const char * const app_xpm[]
#include <X11/extensions/Xrandr.h> 
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <array>
#include <cstring>
#include <chrono>

// External definition check (Assuming g_APP_NAME_A is declared as extern const char* or similar in prc.hxx)
extern const char* g_APP_NAME_A;

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
static unsigned long ParseHexColor(const std::string& colorStr) 
{
    if (colorStr.empty()) return 0;
    if (colorStr[0] == '#') {
        unsigned long hexVal = 0;
        std::stringstream ss(colorStr.substr(1));
        ss >> std::hex >> hexVal;
        return hexVal;
    }
    return 0; // Black fallback for transparent ("None") or unknown values
}

void WaitDialog::Show(const char* txt) 
{
    if (m_running) return;
    m_running = true;
    m_thread = std::thread(&WaitDialog::ThreadLoop, txt);
}

void WaitDialog::ThreadLoop(const char* txt) 
{
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

    // --- DYNAMIC GEOMETRY PARSING VIA XRANDR COMMAND OUTPUT (NO EXTRA LIBS) ---
    m_width = 400;  
    m_height = 150; 

    // Fetch mouse cursor location coordinates to identify the active screen viewport
    Window root_return, child_return;
    int mouse_x = 0, mouse_y = 0;
    int win_x_ret = 0, win_y_ret = 0;
    unsigned int mask_return = 0;
    XQueryPointer(m_display, root, &root_return, &child_return, &mouse_x, &mouse_y, &win_x_ret, &win_y_ret, &mask_return);

    // Initial fallbacks using full virtual root bounding layout bounds
    int active_monitor_x = 0;
    int active_monitor_y = 0;
    int active_monitor_w = DisplayWidth(m_display, screen);
    int active_monitor_h = DisplayHeight(m_display, screen);

    // Execute xrandr system pipe utility asynchronously to fetch hardware topology log lines
    std::array<char, 256> buffer;
    std::unique_ptr<FILE, void(*)(FILE*)> pipe(
        popen("xrandr --current 2>/dev/null", "r"), 
        [](FILE* f) { if (f) pclose(f); }
    );
    
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            std::string line(buffer.data());
            if (line.find(" connected ") != std::string::npos) {
                std::regex geometry_regex(R"((\d+)x(\d+)\+(\d+)\+(\d+))");
                std::smatch match;
                if (std::regex_search(line, match, geometry_regex)) {
                    int w = std::stoi(match[1].str());
                    int h = std::stoi(match[2].str());
                    int x = std::stoi(match[3].str());
                    int y = std::stoi(match[4].str());

                    if (mouse_x >= x && mouse_x < (x + w) && mouse_y >= y && mouse_y < (y + h)) {
                        active_monitor_x = x;
                        active_monitor_y = y;
                        active_monitor_w = w;
                        active_monitor_h = h;
                        break;
                    }
                }
            }
        }
    }

    // Calculate exact mathematical center positioning inside the discovered screen boundaries
    int win_x = active_monitor_x + ((active_monitor_w - m_width) / 2);
    int win_y = active_monitor_y + ((active_monitor_h - m_height) / 2);

    // Block window allocation interference using redirect attributes
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

void WaitDialog::PumpEvents(int frameCounter, const char* baseText) 
{
    if (!m_display || !m_window || !m_pixmap) return;

    XEvent event;
    while (XPending(m_display)) {
        XNextEvent(m_display, &event);
    }

    // Clear window before drawing to avoid text overlapping
    XClearWindow(m_display, m_window);

    // 1. Draw the application icon centered horizontally near the top edge
    int icon_x = (m_width - icon_width) / 2;
    int icon_y = 15; 
    XCopyArea(m_display, m_pixmap, m_window, m_gc, 0, 0, icon_width, icon_height, icon_x, icon_y);

    // 2. Render the stable Application Name string (g_APP_NAME_A) right below the icon
    int app_name_y = icon_y + icon_height + 20; 
    if (g_APP_NAME_A && std::strlen(g_APP_NAME_A) > 0) {
        int app_name_w = std::strlen(g_APP_NAME_A) * 6; // Rough character spacing estimation
        int app_name_x = (m_width - app_name_w) / 2;
        if (app_name_x < 10) app_name_x = 10;
        XDrawString(m_display, m_window, m_gc, app_name_x, app_name_y, g_APP_NAME_A, std::strlen(g_APP_NAME_A));
    }

    // 3. Render the baseText and the separate progressive dots right below the app name
    int status_text_y = app_name_y + 25;
    if (baseText && std::strlen(baseText) > 0) {
        // Build dots string sequence rolling continuously from 0 up to 5 dots
        char dotsStr[10];
        std::memset(dotsStr, 0, sizeof(dotsStr));
        int dotsCount = frameCounter % 6; // Cycles through 0, 1, 2, 3, 4, 5 dots
        for (int i = 0; i < dotsCount; ++i) {
            dotsStr[i] = '.';
        }

        // Fetch the active font structure from GC to calculate precise text width in pixels
        XFontStruct* font_info = XQueryFont(m_display, XGContextFromGC(m_gc));
        int text_width_pixels = 0;
        int space_width_pixels = 6; // Fallback width for a single space character

        if (font_info) {
            text_width_pixels = XTextWidth(font_info, baseText, std::strlen(baseText));
            space_width_pixels = XTextWidth(font_info, " ", 1);
        } else {
            // Hard fallback if font info query fails
            text_width_pixels = std::strlen(baseText) * 6;
        }

        // Calculate the total block width (base text + 1 space spacing + maximum 5 dots padding width)
        int max_dots_width = space_width_pixels + (5 * space_width_pixels);
        int total_combined_block_width = text_width_pixels + max_dots_width;

        // Find the shared starting X coordinate to center the entire block inside the dialog
        int start_x = (m_width - total_combined_block_width) / 2;
        if (start_x < 10) start_x = 10;

        // Draw the UNCHANGED baseText at its fixed, solid position
        XDrawString(m_display, m_window, m_gc, start_x, status_text_y, baseText, std::strlen(baseText));

        // Draw the progressive dots at a fixed X coordinate right behind the text
        int dots_x = start_x + text_width_pixels + space_width_pixels;
        if (dotsCount > 0) {
            XDrawString(m_display, m_window, m_gc, dots_x, status_text_y, dotsStr, std::strlen(dotsStr));
        }

        if (font_info) {
            XFreeFontInfo(nullptr, font_info, 1);
        }
    }

    XFlush(m_display);
}

void WaitDialog::Hide()
{
    if (!m_running)
        return;

    m_running = false;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
