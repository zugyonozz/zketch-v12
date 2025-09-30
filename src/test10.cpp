#include "builtin.hpp"
using namespace zketch;

// Global variables
Rect Client;
InputSystem input;

// Custom drawer untuk slider dengan style berbeda
void CustomSliderDrawer(Canvas* canvas, const Slider& slider) {
    Drawer draw;
    if (!draw.Begin(*canvas)) return;

    // Background transparan
    draw.Clear(rgba(255, 255, 255, 1));
    
    // Track dengan gradient effect
    RectF track = slider.GetRelativeTrackBound();
    if (slider.GetBound().w > slider.GetBound().h) { // Horizontal
        draw.FillRect({track.x, track.y + track.h / 2 - 2, track.w, 4}, rgba(100, 100, 100, 255));
    } else { // Vertical
        draw.FillRect({track.x + track.w / 2 - 2, track.y, 4, track.h}, rgba(100, 100, 100, 255));
    }
    
    // Thumb dengan efek 3D
    RectF thumb = slider.GetRelativeThumbBound();
    float radius = std::min(thumb.w, thumb.h) / 2.0f;
    
    if (slider.IsDragging()) {
        draw.FillCircle({static_cast<int32_t>(thumb.x + thumb.w/2), 
                        static_cast<int32_t>(thumb.y + thumb.h/2)}, 
                       radius, rgba(255, 100, 100, 255));
    } else if (slider.IsHovered()) {
        draw.FillCircle({static_cast<int32_t>(thumb.x + thumb.w/2), 
                        static_cast<int32_t>(thumb.y + thumb.h/2)}, 
                       radius, rgba(135, 206, 250, 255));
    } else {
        draw.FillCircle({static_cast<int32_t>(thumb.x + thumb.w/2), 
                        static_cast<int32_t>(thumb.y + thumb.h/2)}, 
                       radius, rgba(200, 200, 200, 255));
    }
    
    // Border untuk thumb
    draw.DrawCircle({static_cast<int32_t>(thumb.x + thumb.w/2), 
                    static_cast<int32_t>(thumb.y + thumb.h/2)}, 
                   radius, rgba(80, 80, 80, 255), 2.0f);
    
    draw.End();
}

int main() {
    AppRegistry::RegisterWindowClass();
    EventSystem::Initialize();

    Window window("Built-in Widget Demo - Optimized", 900, 600);
    window.Show();

    Client = window.GetClientBound();

    // Create UI Components
    
    // Vertical Slider (Right side)
    Slider vSlider(
        Slider::Vertical, 
        {static_cast<float>(Client.w - 40), 20.0f, 20.0f, static_cast<float>(Client.h - 40)}, 
        {20, 40}
    );
    vSlider.SetRange(0.0f, 100.0f);
    vSlider.SetValue(50.0f);
    vSlider.SetDrawer(CustomSliderDrawer);

    // Horizontal Slider (Bottom)
    Slider hSlider(
        Slider::Horizontal, 
        {20.0f, static_cast<float>(Client.h - 40), static_cast<float>(Client.w - 80), 20.0f}, 
        {40, 20}
    );
    hSlider.SetRange(0.0f, 100.0f);
    hSlider.SetValue(25.0f);
    hSlider.SetDrawer(CustomSliderDrawer);

    // Buttons
    Button btnReset({20, 20, 120, 40}, L"Reset", Font(L"Arial", 14, FontStyle::Bold));
    Button btnApply({160, 20, 120, 40}, L"Apply", Font(L"Arial", 14, FontStyle::Bold));
    Button btnTest({300, 20, 120, 40}, L"Test", Font(L"Arial", 14, FontStyle::Bold));

    // Text displays
    TextBox txtVSlider({440, 20, 200, 40}, L"V-Slider: 50.0", Font(L"Arial", 12, FontStyle::Regular));
    TextBox txtHSlider({440, 70, 200, 40}, L"H-Slider: 25.0", Font(L"Arial", 12, FontStyle::Regular));
    TextBox txtStatus({440, 120, 400, 40}, L"Status: Ready", Font(L"Arial", 12, FontStyle::Italic));

    // Set button callbacks
    btnReset.SetOnClick([&]() {
        vSlider.SetValue(50.0f);
        hSlider.SetValue(25.0f);
        txtVSlider.SetText(L"V-Slider: 50.0");
        txtHSlider.SetText(L"H-Slider: 25.0");
        txtStatus.SetText(L"Status: Reset to defaults");
        logger::info("Reset button clicked!");
    });

    btnApply.SetOnClick([&]() {
        std::wstring msg = L"Status: Applied V=" + 
                          std::to_wstring(static_cast<int>(vSlider.GetValue())) + 
                          L" H=" + 
                          std::to_wstring(static_cast<int>(hSlider.GetValue()));
        txtStatus.SetText(msg);
        logger::info("Apply button clicked!");
    });

    btnTest.SetOnClick([&]() {
        static int counter = 0;
        counter++;
        txtStatus.SetText(L"Status: Test clicked " + std::to_wstring(counter) + L" times");
        logger::info("Test button clicked! Count: ", counter);
    });

    Event e;
    bool mouse_down = false;

    logger::info("=== Demo Started ===");
    logger::info("Instructions:");
    logger::info("- Drag the sliders with mouse");
    logger::info("- Click buttons to see effects");
    logger::info("- Close window to exit");

    while (Application) {
        input.Update();

        while (PollEvent(e)) {
            // Handle Mouse Events
            if (e == EventType::Mouse) {
                PointF mouse_pos = e.GetMousePosition();

                if (e.GetMouseState() == MouseState::Down && 
                    e.GetMouseButton() == MouseButton::Left) {
                    mouse_down = true;
                    
                    // Try press on widgets
                    vSlider.OnPress(mouse_pos);
                    hSlider.OnPress(mouse_pos);
                    btnReset.OnPress(mouse_pos);
                    btnApply.OnPress(mouse_pos);
                    btnTest.OnPress(mouse_pos);
                    
                } else if (e.GetMouseState() == MouseState::Up && 
                          e.GetMouseButton() == MouseButton::Left) {
                    mouse_down = false;
                    
                    // Release widgets
                    vSlider.OnRelease();
                    hSlider.OnRelease();
                    btnReset.OnRelease(mouse_pos);
                    btnApply.OnRelease(mouse_pos);
                    btnTest.OnRelease(mouse_pos);
                    
                } else if (e.GetMouseState() == MouseState::None) {
                    // Hover detection
                    vSlider.OnHover(mouse_pos);
                    hSlider.OnHover(mouse_pos);
                    btnReset.OnHover(mouse_pos);
                    btnApply.OnHover(mouse_pos);
                    btnTest.OnHover(mouse_pos);
                    
                    // Drag if mouse is down
                    if (mouse_down) {
                        vSlider.OnDrag(mouse_pos);
                        hSlider.OnDrag(mouse_pos);
                    }
                }
            }

            // Handle Slider Events
            if (e == EventType::Slider) {
                if (e.GetSliderState() == SliderState::Changed) {
                    float value = e.GetSliderValue();
                    
                    // Update text based on which slider changed
                    if (vSlider.IsDragging()) {
                        std::wstring text = L"V-Slider: " + 
                                          std::to_wstring(static_cast<int>(vSlider.GetValue()));
                        txtVSlider.SetText(text);
                    } else if (hSlider.IsDragging()) {
                        std::wstring text = L"H-Slider: " + 
                                          std::to_wstring(static_cast<int>(hSlider.GetValue()));
                        txtHSlider.SetText(text);
                    }
                }
            }

            // Handle Resize
            if (e == EventType::Resize) {
                Client = window.GetClientBound();
                
                // Reposition vertical slider
                vSlider.SetPosition({static_cast<float>(Client.w - 40), 20.0f});
                
                // Update all widgets
                vSlider.MarkDirty();
                hSlider.MarkDirty();
                btnReset.MarkDirty();
                btnApply.MarkDirty();
                btnTest.MarkDirty();
                txtVSlider.MarkDirty();
                txtHSlider.MarkDirty();
                txtStatus.MarkDirty();
                
                logger::info("Window resized to: ", Client.w, "x", Client.h);
            }

            // Handle Close
            if (e == EventType::Close) {
                logger::info("Close event received");
            }
        }

        // Present all widgets
        HWND hwnd = window.GetHandle();
        
        vSlider.Present(hwnd);
        hSlider.Present(hwnd);
        btnReset.Present(hwnd);
        btnApply.Present(hwnd);
        btnTest.Present(hwnd);
        txtVSlider.Present(hwnd);
        txtHSlider.Present(hwnd);
        txtStatus.Present(hwnd);

        Sleep(16); // ~60 FPS
    }

    logger::info("=== Demo Ended ===");
    return 0;
}