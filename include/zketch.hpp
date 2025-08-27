#pragma once

// Unified zketch framework - simplified and optimized
#include <algorithm>
#include <atomic>
#include <bitset>
#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace zketch {

//=============================================================================
// CORE TYPES & UTILITIES
//=============================================================================

using str = std::string;
template<typename T> using uptr = std::unique_ptr<T>;
template<typename K, typename V> using fast_map = std::unordered_map<K, V>;

// High precision timer
class Timer {
    std::chrono::high_resolution_clock::time_point start_;
public:
    Timer() { reset(); }
    void reset() { start_ = std::chrono::high_resolution_clock::now(); }
    double seconds() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(now - start_).count();
    }
    double milliseconds() const { return seconds() * 1000.0; }
};

//=============================================================================
// MATH & GEOMETRY
//=============================================================================

struct Point {
    float x = 0, y = 0;
    
    Point() = default;
    Point(float v) : x(v), y(v) {}
    Point(float x, float y) : x(x), y(y) {}
    
    Point operator+(const Point& p) const { return {x + p.x, y + p.y}; }
    Point operator-(const Point& p) const { return {x - p.x, y - p.y}; }
    Point operator*(float s) const { return {x * s, y * s}; }
    Point operator/(float s) const { return {x / s, y / s}; }
    
    Point& operator+=(const Point& p) { x += p.x; y += p.y; return *this; }
    Point& operator-=(const Point& p) { x -= p.x; y -= p.y; return *this; }
    Point& operator*=(float s) { x *= s; y *= s; return *this; }
    Point& operator/=(float s) { x /= s; y /= s; return *this; }
    
    bool operator==(const Point& p) const { return x == p.x && y == p.y; }
    
    float length() const { return std::sqrt(x*x + y*y); }
    Point normalized() const { float l = length(); return l > 0 ? *this / l : Point{}; }
    float dot(const Point& p) const { return x * p.x + y * p.y; }
    
    operator Gdiplus::PointF() const { return {x, y}; }
};

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    
    Rect() = default;
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    Rect(const Point& pos, const Point& size) : x(pos.x), y(pos.y), w(size.x), h(size.y) {}
    
    Point pos() const { return {x, y}; }
    Point size() const { return {w, h}; }
    Point center() const { return {x + w*0.5f, y + h*0.5f}; }
    
    bool contains(const Point& p) const { 
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h; 
    }
    
    bool intersects(const Rect& r) const {
        return !(x + w < r.x || r.x + r.w < x || y + h < r.y || r.y + r.h < y);
    }
    
    operator Gdiplus::RectF() const { return {x, y, w, h}; }
};

//=============================================================================
// COLOR SYSTEM
//=============================================================================

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
    
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    Color(uint32_t rgba) : r(rgba & 0xFF), g((rgba >> 8) & 0xFF), 
                           b((rgba >> 16) & 0xFF), a((rgba >> 24) & 0xFF) {}
    
    static Color Red() { return {255, 0, 0}; }
    static Color Green() { return {0, 255, 0}; }
    static Color Blue() { return {0, 0, 255}; }
    static Color White() { return {255, 255, 255}; }
    static Color Black() { return {0, 0, 0}; }
    static Color Transparent() { return {0, 0, 0, 0}; }
    
    Color with_alpha(uint8_t alpha) const { return {r, g, b, alpha}; }
    
    operator Gdiplus::Color() const { return Gdiplus::Color(a, r, g, b); }
    operator COLORREF() const { return RGB(r, g, b); }
};

//=============================================================================
// FONT SYSTEM  
//=============================================================================

enum class FontStyle { Regular = 0, Bold = 1, Italic = 2, BoldItalic = 3 };

struct Font {
    std::wstring family = L"Arial";
    float size = 12.0f;
    FontStyle style = FontStyle::Regular;
    
    Font() = default;
    Font(const std::wstring& family, float size, FontStyle style = FontStyle::Regular) 
        : family(family), size(size), style(style) {}
};

//=============================================================================
// EVENT SYSTEM
//=============================================================================

enum class EventType { None, Quit, KeyDown, KeyUp, MouseMove, MouseDown, MouseUp, Resize, Close };
enum class MouseButton { Left, Right, Middle, Unknown };

struct Event {
    EventType type = EventType::None;
    union {
        struct { int keyCode; } key;
        struct { Point pos; MouseButton button; } mouse;
        struct { Point size; } resize;
    };
    
    Event() { key.keyCode = 0; } // Initialize union
};

//=============================================================================
// INPUT MANAGEMENT
//=============================================================================

class Input {
    std::bitset<256> keys_down_, keys_pressed_, keys_released_;
    std::bitset<3> mouse_down_, mouse_pressed_, mouse_released_;
    Point mouse_pos_, mouse_delta_;
    
public:
    void update() {
        keys_pressed_.reset();
        keys_released_.reset();
        mouse_pressed_.reset();
        mouse_released_.reset();
        mouse_delta_ = {};
    }
    
    void set_key_down(int key) {
        if (key >= 0 && key < 256 && !keys_down_[key]) keys_pressed_[key] = true;
        if (key >= 0 && key < 256) keys_down_[key] = true;
    }
    
    void set_key_up(int key) {
        if (key >= 0 && key < 256 && keys_down_[key]) keys_released_[key] = true;
        if (key >= 0 && key < 256) keys_down_[key] = false;
    }
    
    void set_mouse_down(int button) {
        if (button >= 0 && button < 3 && !mouse_down_[button]) mouse_pressed_[button] = true;
        if (button >= 0 && button < 3) mouse_down_[button] = true;
    }
    
    void set_mouse_up(int button) {
        if (button >= 0 && button < 3 && mouse_down_[button]) mouse_released_[button] = true;
        if (button >= 0 && button < 3) mouse_down_[button] = false;
    }
    
    void set_mouse_pos(const Point& pos) {
        mouse_delta_ = pos - mouse_pos_;
        mouse_pos_ = pos;
    }
    
    bool key_down(int key) const { return key >= 0 && key < 256 && keys_down_[key]; }
    bool key_pressed(int key) const { return key >= 0 && key < 256 && keys_pressed_[key]; }
    bool key_released(int key) const { return key >= 0 && key < 256 && keys_released_[key]; }
    
    bool mouse_down(int button) const { return button >= 0 && button < 3 && mouse_down_[button]; }
    bool mouse_pressed(int button) const { return button >= 0 && button < 3 && mouse_pressed_[button]; }
    bool mouse_released(int button) const { return button >= 0 && button < 3 && mouse_released_[button]; }
    
    Point mouse_pos() const { return mouse_pos_; }
    Point mouse_delta() const { return mouse_delta_; }
};

//=============================================================================
// ANIMATION SYSTEM
//=============================================================================

enum class Easing { Linear, QuadIn, QuadOut, QuadInOut, CubicIn, CubicOut, BounceOut };

class Animator {
    struct Anim {
        float start, end, duration, elapsed;
        Easing easing;
        std::function<void(float)> callback;
        bool active = false;
    };
    std::vector<Anim> anims_;
    
    static float ease(float t, Easing type) {
        switch (type) {
            case Easing::Linear: return t;
            case Easing::QuadIn: return t * t;
            case Easing::QuadOut: return 1.0f - (1.0f - t) * (1.0f - t);
            case Easing::QuadInOut: return t < 0.5f ? 2*t*t : 1-2*(1-t)*(1-t);
            case Easing::CubicIn: return t * t * t;
            case Easing::CubicOut: return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
            case Easing::BounceOut: {
				if (t < 1/2.75f) {
					return 7.5625f * t * t;
				} else if (t < 2/2.75f) {
					float u = t - 1.5f/2.75f;
					return 7.5625f * u * u + 0.75f;
				} else if (t < 2.5f/2.75f) {
					float u = t - 2.25f/2.75f;
					return 7.5625f * u * u + 0.9375f;
				} else {
					float u = t - 2.625f/2.75f;
					return 7.5625f * u * u + 0.984375f;
				}
			}
            default: return t;
        }
    }
    
public:
    void animate(float from, float to, float duration, Easing easing, std::function<void(float)> callback) {
        anims_.push_back({from, to, duration, 0, easing, callback, true});
    }
    
    void update(float dt) {
        for (auto& a : anims_) {
            if (!a.active) continue;
            a.elapsed += dt;
            float t = std::min(a.elapsed / a.duration, 1.0f);
            float value = a.start + (a.end - a.start) * ease(t, a.easing);
            a.callback(value);
            if (t >= 1.0f) a.active = false;
        }
        anims_.erase(std::remove_if(anims_.begin(), anims_.end(), 
            [](const Anim& a) { return !a.active; }), anims_.end());
    }
    
    void clear() { anims_.clear(); }
};

//=============================================================================
// GRAPHICS SYSTEM
//=============================================================================

// GDI+ RAII wrapper
class GDISession {
    ULONG_PTR token_;
public:
    GDISession() {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&token_, &input, nullptr);
    }
    ~GDISession() { Gdiplus::GdiplusShutdown(token_); }
};
static GDISession gdi_session; // Global instance

class Graphics {
    uptr<Gdiplus::Graphics> gfx_;
    uptr<Gdiplus::Bitmap> target_;
    
public:
    bool begin(const Point& size) {
        target_ = std::make_unique<Gdiplus::Bitmap>(int(size.x), int(size.y), PixelFormat32bppARGB);
        gfx_ = std::make_unique<Gdiplus::Graphics>(target_.get());
        if (gfx_ && gfx_->GetLastStatus() == Gdiplus::Ok) {
            gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            return true;
        }
        return false;
    }
    
    void end() { gfx_.reset(); }
    
    void present(HDC hdc) const {
        if (target_) {
            Gdiplus::Graphics screen(hdc);
            screen.DrawImage(target_.get(), 0, 0);
        }
    }
    
    void clear(const Color& color = Color::Black()) {
        if (gfx_) gfx_->Clear(color);
    }
    
    // Drawing methods - simplified interface
    void fill_rect(const Rect& rect, const Color& color) {
        if (!gfx_) return;
        Gdiplus::SolidBrush brush(color);
        gfx_->FillRectangle(&brush, rect);
    }
    
    void draw_rect(const Rect& rect, const Color& color, float thickness = 1.0f) {
        if (!gfx_) return;
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawRectangle(&pen, rect);
    }
    
    void fill_rounded_rect(const Rect& rect, float radius, const Color& color) {
        if (!gfx_ || radius <= 0) return;
        Gdiplus::GraphicsPath path;
        float d = radius * 2;
        path.AddArc(rect.x, rect.y, d, d, 180, 90);
        path.AddArc(rect.x + rect.w - d, rect.y, d, d, 270, 90);
        path.AddArc(rect.x + rect.w - d, rect.y + rect.h - d, d, d, 0, 90);
        path.AddArc(rect.x, rect.y + rect.h - d, d, d, 90, 90);
        path.CloseFigure();
        Gdiplus::SolidBrush brush(color);
        gfx_->FillPath(&brush, &path);
    }
    
    void draw_rounded_rect(const Rect& rect, float radius, const Color& color, float thickness = 1.0f) {
        if (!gfx_ || radius <= 0) return;
        Gdiplus::GraphicsPath path;
        float d = radius * 2;
        path.AddArc(rect.x, rect.y, d, d, 180, 90);
        path.AddArc(rect.x + rect.w - d, rect.y, d, d, 270, 90);
        path.AddArc(rect.x + rect.w - d, rect.y + rect.h - d, d, d, 0, 90);
        path.AddArc(rect.x, rect.y + rect.h - d, d, d, 90, 90);
        path.CloseFigure();
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawPath(&pen, &path);
    }
    
    void fill_circle(const Point& center, float radius, const Color& color) {
        if (!gfx_) return;
        Gdiplus::SolidBrush brush(color);
        gfx_->FillEllipse(&brush, center.x - radius, center.y - radius, radius * 2, radius * 2);
    }
    
    void draw_circle(const Point& center, float radius, const Color& color, float thickness = 1.0f) {
        if (!gfx_) return;
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawEllipse(&pen, center.x - radius, center.y - radius, radius * 2, radius * 2);
    }
    
    void draw_line(const Point& start, const Point& end, const Color& color, float thickness = 1.0f) {
        if (!gfx_) return;
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawLine(&pen, start, end);
    }
    
    void draw_text(const std::wstring& text, const Point& pos, const Font& font, const Color& color) {
        if (!gfx_) return;
        Gdiplus::FontFamily family(font.family.c_str());
        Gdiplus::Font gdiFont(&family, font.size, int(font.style), Gdiplus::UnitPixel);
        Gdiplus::SolidBrush brush(color);
        gfx_->DrawString(text.c_str(), -1, &gdiFont, pos, &brush);
    }
    
    bool is_drawing() const { return gfx_ != nullptr; }
};

//=============================================================================
// WINDOW SYSTEM
//=============================================================================

class Window {
    HWND hwnd_ = nullptr;
    Rect bounds_;
    str title_;
    str class_name_;
    std::queue<Event> events_;
    std::atomic<bool> should_close_{false};
    
    Graphics gfx_;
    Input input_;
    Timer timer_;
    Animator animator_;
    
    static fast_map<HWND, Window*> windows_;
    static std::atomic<int> window_count_;
    
    bool register_class() {
        class_name_ = "zketch_window_" + std::to_string(reinterpret_cast<uintptr_t>(this));
        
        WNDCLASSEXA wc{};
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = window_proc;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = class_name_.c_str();
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        
        return RegisterClassExA(&wc) != 0;
    }
    
    void create_window() {
        hwnd_ = CreateWindowExA(0, class_name_.c_str(), title_.c_str(), 
                               WS_OVERLAPPEDWINDOW, int(bounds_.x), int(bounds_.y),
                               int(bounds_.w), int(bounds_.h), nullptr, nullptr,
                               GetModuleHandleA(nullptr), this);
        if (hwnd_) {
            windows_[hwnd_] = this;
            window_count_++;
        }
    }
    
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        Window* window = nullptr;
        
        if (msg == WM_NCCREATE) {
            auto cs = reinterpret_cast<CREATESTRUCTA*>(lp);
            window = static_cast<Window*>(cs->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        } else {
            window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }
        
        if (window) return window->handle_message(hwnd, msg, wp, lp);
        return DefWindowProcA(hwnd, msg, wp, lp);
    }
    
    LRESULT handle_message(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        Event event;
        
        switch (msg) {
            case WM_CLOSE:
                should_close_ = true;
                event.type = EventType::Close;
                events_.push(event);
                return 0;
                
            case WM_DESTROY:
                windows_.erase(hwnd_);
                if (--window_count_ == 0) PostQuitMessage(0);
                return 0;
                
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                on_paint();
                gfx_.present(hdc);
                EndPaint(hwnd, &ps);
                return 0;
            }
                
            case WM_SIZE:
                bounds_.w = LOWORD(lp);
                bounds_.h = HIWORD(lp);
                event.type = EventType::Resize;
                event.resize.size = {float(LOWORD(lp)), float(HIWORD(lp))};
                events_.push(event);
                on_resize(event.resize.size);
                return 0;
                
            case WM_KEYDOWN:
                input_.set_key_down(int(wp));
                event.type = EventType::KeyDown;
                event.key.keyCode = int(wp);
                events_.push(event);
                return 0;
                
            case WM_KEYUP:
                input_.set_key_up(int(wp));
                event.type = EventType::KeyUp;
                event.key.keyCode = int(wp);
                events_.push(event);
                return 0;
                
            case WM_MOUSEMOVE:
                input_.set_mouse_pos({float(GET_X_LPARAM(lp)), float(GET_Y_LPARAM(lp))});
                event.type = EventType::MouseMove;
                event.mouse.pos = {float(GET_X_LPARAM(lp)), float(GET_Y_LPARAM(lp))};
                events_.push(event);
                return 0;
                
            case WM_LBUTTONDOWN:
                input_.set_mouse_down(0);
                event.type = EventType::MouseDown;
                event.mouse.pos = {float(GET_X_LPARAM(lp)), float(GET_Y_LPARAM(lp))};
                event.mouse.button = MouseButton::Left;
                events_.push(event);
                return 0;
                
            case WM_LBUTTONUP:
                input_.set_mouse_up(0);
                event.type = EventType::MouseUp;
                event.mouse.pos = {float(GET_X_LPARAM(lp)), float(GET_Y_LPARAM(lp))};
                event.mouse.button = MouseButton::Left;
                events_.push(event);
                return 0;
        }
        
        return DefWindowProcA(hwnd, msg, wp, lp);
    }
    
protected:
    virtual void on_paint() {}
    virtual void on_resize(const Point& size) {}
    virtual void on_update(float dt) {}
    
public:
    Window(const str& title, const Point& size, const Point& pos = {100, 100}) 
        : title_(title), bounds_(pos.x, pos.y, size.x, size.y) {
        if (register_class()) {
            create_window();
            gfx_.begin(size);
        }
    }
    
    virtual ~Window() {
        if (hwnd_) {
            DestroyWindow(hwnd_);
            hwnd_ = nullptr;
        }
    }
    
    // Main interface - simplified
    void show() { if (hwnd_) ShowWindow(hwnd_, SW_SHOW); }
    void hide() { if (hwnd_) ShowWindow(hwnd_, SW_HIDE); }
    void close() { should_close_ = true; }
    
    bool should_close() const { return should_close_; }
    bool poll_event(Event& event) {
        if (!events_.empty()) {
            event = events_.front();
            events_.pop();
            return true;
        }
        return false;
    }
    
    // Graphics access - simplified
    Graphics& draw() { return gfx_; }
    Input& input() { return input_; }
    Animator& animate() { return animator_; }
    Timer& timer() { return timer_; }
    
    Point size() const { return bounds_.size(); }
    Rect bounds() const { return bounds_; }
    
    void invalidate() { if (hwnd_) InvalidateRect(hwnd_, nullptr, FALSE); }
    
    static bool process_messages() {
        MSG msg;
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return false;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return true;
    }
    
    // Update cycle - call this in your main loop
    void update() {
        float dt = float(timer_.milliseconds()) / 1000.0f;
        timer_.reset();
        
        animator_.update(dt);
        input_.update();
        on_update(dt);
        
        invalidate(); // Trigger repaint
    }
};

// Static member definitions
fast_map<HWND, Window*> Window::windows_;
std::atomic<int> Window::window_count_{0};

//=============================================================================
// CONVENIENCE FUNCTIONS - Make everything even easier
//=============================================================================

// Quick app creation
inline uptr<Window> create_window(const str& title, int width, int height) {
    return std::make_unique<Window>(title, Point{float(width), float(height)});
}

// Color helpers
namespace colors {
    inline Color rgb(int r, int g, int b) { return {uint8_t(r), uint8_t(g), uint8_t(b)}; }
    inline Color rgba(int r, int g, int b, int a) { return {uint8_t(r), uint8_t(g), uint8_t(b), uint8_t(a)}; }
    inline Color hex(uint32_t hex) { return Color(hex); }
}

// Font helpers
inline Font font(const str& family, float size, FontStyle style = FontStyle::Regular) {
    std::wstring wfamily(family.begin(), family.end());
    return {wfamily, size, style};
}

// Animation helpers
template<typename T>
inline void animate_to(Animator& anim, T& value, T target, float duration, Easing easing = Easing::QuadOut) {
    T start = value;
    anim.animate(0, 1, duration, easing, [&value, start, target](float t) {
        if constexpr (std::is_same_v<T, float>) {
            value = start + (target - start) * t;
        } else if constexpr (std::is_same_v<T, Point>) {
            value = start + (target - start) * t;
        }
    });
}

} // namespace zketch