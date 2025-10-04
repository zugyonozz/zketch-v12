#include "zketch.hpp"
using namespace zketch ;

// Simple demo untuk testing dasar
int main() {
    zketch_init() ;
    
    using namespace zketch ;
    
    // Create window
    Window window("Simple zketch Demo", 800, 600) ;
    if (!window.IsValid()) {
        logger::error("Failed to create window!") ;
        return 1 ;
    }
    
    // Create canvas
    Canvas canvas ;
    Rect client = window.GetClientBound() ;
    if (!canvas.Create(client.GetSize())) {
        logger::error("Failed to create canvas!") ;
        return 1 ;
    }
    canvas.SetClearColor(rgba(250, 250, 250, 255)) ;
    
    // Create button
    Button button(
        RectF{300, 250, 200, 80},
        L"Click Me!",
        Font(L"Arial", 16, FontStyle::Bold)
    ) ;
    
    int click_count = 0 ;
    button.SetOnClick([&]() {
        click_count++ ;
        logger::info("Button clicked! Count: ", click_count) ;
    }) ;
    
    // Create slider
    Slider slider(
        Slider::Horizontal,
        RectF{250, 400, 300, 10},
        SizeF{20, 30}
    ) ;
    slider.SetRange(0, 100) ;
    slider.SetValue(50) ;
    
    // Input system
    InputSystem input ;
    
    window.Show() ;
    logger::info("Simple demo started. Press ESC to quit.") ;
    
    while (Application) {
        Event event ;
        while (PollEvent(event)) {
            if (event == EventType::Quit || event == EventType::Close) {
                Application.QuitProgram() ;
                return 0 ;
            }
            
            if (event.IsKeyEvent()) {
                if (event.GetKeyState() == KeyState::Down) {
                    input.SetKeyDown(event.GetKeyCode()) ;
                    
                    if (event.GetKeyCode() == VK_ESCAPE) {
                        window.Quit() ;
                    }
                } else {
                    input.SetKeyUp(event.GetKeyCode()) ;
                }
            } else if (event.IsMouseEvent()) {
                Point pos = event.GetMousePosition() ;
                PointF posf = {static_cast<float>(pos.x), static_cast<float>(pos.y)} ;
                
                input.SetMousePos(pos) ;
                
                if (event.GetMouseState() == MouseState::Down) {
                    button.OnPress(posf) ;
                    slider.OnPress(posf) ;
                } else if (event.GetMouseState() == MouseState::Up) {
                    button.OnRelease(posf) ;
                    slider.OnRelease() ;
                } else if (event.GetMouseState() == MouseState::None) {
                    button.OnHover(posf) ;
                    slider.OnHover(posf) ;
                    slider.OnDrag(posf) ;
                }
            }
        }
        
        // Render
        Renderer renderer ;
        if (renderer.Begin(canvas)) {
            // Draw background gradient
            renderer.FillRect(
                RectF{0, 0, static_cast<float>(canvas.GetWidth()), static_cast<float>(canvas.GetHeight())},
                rgba(245, 245, 250, 255)
            ) ;
            
            // Draw title
            renderer.DrawString(
                L"Simple zketch Demo",
                {250, 100},
                rgba(60, 60, 60, 255),
                Font(L"Arial", 28, FontStyle::Bold)
            ) ;
            
            // Draw click counter
            std::wstring counter_text = L"Clicks: " + std::to_wstring(click_count) ;
            renderer.DrawString(
                counter_text,
                {320, 180},
                rgba(33, 150, 243, 255),
                Font(L"Consolas", 20, FontStyle::Bold)
            ) ;
            
            // Draw slider value
            std::wstring slider_text = L"Slider: " + std::to_wstring(static_cast<int>(slider.GetValue())) ;
            renderer.DrawString(
                slider_text,
                {350, 450},
                rgba(76, 175, 80, 255),
                Font(L"Consolas", 16, FontStyle::Regular)
            ) ;
            
            // Draw some shapes
            renderer.FillCircle({150, 300}, 50, rgba(255, 87, 34, 200)) ;
            renderer.DrawCircle({150, 300}, 50, rgba(230, 74, 25, 255), 3.0f) ;
            
            renderer.FillRectRounded(
                RectF{600, 250, 100, 100},
                rgba(156, 39, 176, 200),
                15.0f
            ) ;
            
            renderer.End() ;
        }
        
        // Present widgets
        button.Present(window) ;
        slider.Present(window) ;
        
        // Present main canvas
        canvas.Present(window) ;
        
        input.Update() ;
        Sleep(16) ; // ~60 FPS
    }
    
    logger::info("Simple demo ended.") ;
    return 0 ;
}