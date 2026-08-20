//
// WaitDialog.cxx
//

#include "prc.hxx"
#include "WaitDialog.hxx"
#include "../resources/app.xpm" // Provides static const char * const app_xpm[]
#include <X11/extensions/Xrandr.h> 

// External application name token defined in prc.hxx
extern const char* g_APP_NAME_A;

// FIXED: All static allocations and definitions defined purely here without inline headers
std::thread       WaitDialog::m_thread;
std::atomic<bool> WaitDialog::m_running{false}; // Initialized out-of-line safely
std::string       WaitDialog::m_current_text;
std::mutex        WaitDialog::m_text_mutex;

Display*          WaitDialog::m_display = nullptr;
Window            WaitDialog::m_window  = 0;
Pixmap            WaitDialog::m_pixmap  = 0;
GC                WaitDialog::m_gc      = nullptr;
int               WaitDialog::m_width   = 400; 
int               WaitDialog::m_height  = 150; 

static int icon_width = 0;
static int icon_height = 0;

// Helper function to parse hex colors manually from XPM strings
static unsigned long ParseHexColor(const std::string& colorStr, unsigned long defaultColor = 0) 
{
    if (colorStr.empty() || colorStr == "None") return defaultColor;
    if (colorStr[0] == '#') {
        unsigned long hexVal = 0;
        std::stringstream ss(colorStr.substr(1));
        ss >> std::hex >> hexVal;
        return hexVal;
    }
    return defaultColor; 
}

void WaitDialog::Show(const char* txt) 
{
    if (m_running) return;
    
    {
        std::lock_guard<std::mutex> lock(m_text_mutex);
        m_current_text = txt ? txt : "";
    }

    m_running = true;
    m_thread = std::thread(&WaitDialog::ThreadLoop);
}

void WaitDialog::SetText(const char* txt)
{
    if (!m_running) return;
    std::lock_guard<std::mutex> lock(m_text_mutex);
    m_current_text = txt ? txt : "";
}

void WaitDialog::ThreadLoop() 
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

    Colormap colormap = DefaultColormap(m_display, screen);
    XColor bgColor;
    unsigned long bgPixel = 0x6f6f6f;
    if (XParseColor(m_display, colormap, "#6f6f6f", &bgColor) && XAllocColor(m_display, colormap, &bgColor)) {
        bgPixel = bgColor.pixel;
    }

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
            colorMap[key] = ParseHexColor(colorVal, bgPixel);
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

    // --- DYNAMIC GEOMETRY PARSING VIA XRANDR COMMAND OUTPUT ---
    m_width = 400;  
    m_height = 150; 

    Window root_return, child_return;
    int mouse_x = 0, mouse_y = 0;
    int win_x_ret = 0, win_y_ret = 0;
    unsigned int mask_return = 0;
    XQueryPointer(m_display, root, &root_return, &child_return, &mouse_x, &mouse_y, &win_x_ret, &win_y_ret, &mask_return);

    int active_monitor_x = 0;
    int active_monitor_y = 0;
    int active_monitor_w = DisplayWidth(m_display, screen);
    int active_monitor_h = DisplayHeight(m_display, screen);

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

    int win_x = active_monitor_x + ((active_monitor_w - m_width) / 2);
    int win_y = active_monitor_y + ((active_monitor_h - m_height) / 2);

    XSetWindowAttributes window_attributes;
    window_attributes.override_redirect = True; 
    window_attributes.background_pixel = bgPixel;

    m_window = XCreateWindow(
        m_display, root, win_x, win_y, m_width, m_height, 0,
        depth, InputOutput, visual,
        CWOverrideRedirect | CWBackPixel, &window_attributes
    );

    Atom window_type = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE", False);
    Atom type_splash = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    XChangeProperty(m_display, m_window, window_type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&type_splash), 1);

    XMapWindow(m_display, m_window);
    XFlush(m_display);

    m_gc = XCreateGC(m_display, m_window, 0, nullptr);
    XSetForeground(m_display, m_gc, WhitePixel(m_display, screen));

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

    while (m_running) {
        PumpEvents(frameCounter);
        frameCounter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (m_gc) XFreeGC(m_display, m_gc);
    if (m_pixmap) XFreePixmap(m_display, m_pixmap);
    if (m_window) XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
    
    m_display = nullptr;
    m_window = 0;
    m_pixmap = 0;
    m_gc = nullptr;
}

void WaitDialog::PumpEvents(int frameCounter) 
{
    if (!m_display || !m_window || !m_pixmap) return;

    XEvent event;
    while (XPending(m_display)) {
        XNextEvent(m_display, &event);
    }

    XClearWindow(m_display, m_window);

    // 1. Draw Application Icon
    int icon_x = (m_width - icon_width) / 2;
    int icon_y = 15; 
    XCopyArea(m_display, m_pixmap, m_window, m_gc, 0, 0, icon_width, icon_height, icon_x, icon_y);

    // 2. Render Application Name (g_APP_NAME_A)
    int app_name_y = icon_y + icon_height + 20; 
    if (g_APP_NAME_A && std::strlen(g_APP_NAME_A) > 0) {
        int app_name_w = std::strlen(g_APP_NAME_A) * 6; 
        int app_name_x = (m_width - app_name_w) / 2;
        if (app_name_x < 10) app_name_x = 10;
        XDrawString(m_display, m_window, m_gc, app_name_x, app_name_y, g_APP_NAME_A, std::strlen(g_APP_NAME_A));
        XDrawString(m_display, m_window, m_gc, app_name_x + 1, app_name_y, g_APP_NAME_A, std::strlen(g_APP_NAME_A));
    }

    // 3. Fetch current status text thread-safely and render it
    int status_text_y = app_name_y + 25;
    
    std::string local_text;
    {
        std::lock_guard<std::mutex> lock(m_text_mutex);
        local_text = m_current_text;
    }

    if (!local_text.empty()) {
        char dotsStr[10];
        std::memset(dotsStr, 0, sizeof(dotsStr));
        int dotsCount = frameCounter % 6; 
        for (int i = 0; i < dotsCount; ++i) {
            dotsStr[i] = '.';
        }

        XFontStruct* font_info = XQueryFont(m_display, XGContextFromGC(m_gc));
        int text_width_pixels = 0;
        int space_width_pixels = 6; 

        if (font_info) {
            text_width_pixels = XTextWidth(font_info, local_text.c_str(), local_text.length());
            space_width_pixels = XTextWidth(font_info, " ", 1);
        } else {
            text_width_pixels = local_text.length() * 6;
        }

        int max_dots_width = space_width_pixels + (5 * space_width_pixels);
        int total_combined_block_width = text_width_pixels + max_dots_width;

        int start_x = (m_width - total_combined_block_width) / 2;
        if (start_x < 10) start_x = 10;

        XDrawString(m_display, m_window, m_gc, start_x, status_text_y, local_text.c_str(), local_text.length());
        XDrawString(m_display, m_window, m_gc, start_x + 1, status_text_y, local_text.c_str(), local_text.length());

        int dots_x = start_x + text_width_pixels + space_width_pixels;
        if (dotsCount > 0) {
            XDrawString(m_display, m_window, m_gc, dots_x, status_text_y, dotsStr, std::strlen(dotsStr));
            XDrawString(m_display, m_window, m_gc, dots_x + 1, status_text_y, dotsStr, std::strlen(dotsStr));
        }

        if (font_info) {
            XFreeFontInfo(nullptr, font_info, 1);
        }
    }

    XFlush(m_display);
}

void WaitDialog::Hide() 
{
    if (!m_running) return;
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
