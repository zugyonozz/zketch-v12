#include "slider.hpp"
using namespace zketch ;

int main() {
    AppRegistry::RegisterWindowClass() ;

    Window window("Slider Demo", 900, 800) ;
    window.Show() ;

    RectF client = window.GetClientBound() ;
    
    // Create multiple sliders for testing
    Slider vertical_slider(Slider::Vertical, {client.w - 30, 20, 25, client.h - 100}, 0, 100, 60) ;
    Slider horizontal_slider(Slider::Horizontal, {50, client.h - 50, client.w - 100, 25}, 0, 100, 30) ;

    // Customize vertical slider
    auto& v_style = vertical_slider.GetStyle();
    v_style.thumb_fill = Color(255, 100, 100, 255);      // Red thumb
    v_style.thumb_hover = Color(255, 150, 150, 255);     // Light red on hover
    v_style.track_fill = Color(100, 100, 255, 255);      // Blue track
    v_style.thumb_size = 20.0f;                          // Bigger thumb
    v_style.track_thickness = 8.0f;                      // Thicker track

    // Customize horizontal slider
    auto& h_style = horizontal_slider.GetStyle();
    h_style.thumb_fill = Color(100, 255, 100, 255);      // Green thumb
    h_style.thumb_hover = Color(150, 255, 150, 255);     // Light green on hover
    h_style.track_fill = Color(255, 100, 255, 255);      // Purple track
    h_style.thumb_size = 18.0f;                          
    h_style.track_thickness = 6.0f;                      

    InputSystem input ;

    while (Application) {
        input.Update() ;
        
        Event e ;
        while (PollEvent(e)) {
            switch (e) {
                case EventType::MouseDown:
                    if (e.GetMouseButton() == MouseButton::Left) {
                        Point mouse_pos = e.GetMousePos() ;
                        PointF mouse_posf = {static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)} ;
                        
                        // Try vertical slider first
                        if (!vertical_slider.OnMouseDown(mouse_posf)) {
                            // If vertical slider didn't handle it, try horizontal
                            horizontal_slider.OnMouseDown(mouse_posf) ;
                        }
                        input.SetMouseDown(1) ; // Left button
                        input.SetMousePos(mouse_pos) ;
                    }
                    break ;

                case EventType::MouseMove:
                    {
                        Point mouse_pos = e.GetMousePos() ;
                        PointF mouse_posf = {static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)} ;
                        
                        vertical_slider.OnMouseMove(mouse_posf) ;
                        horizontal_slider.OnMouseMove(mouse_posf) ;
                        input.SetMousePos(mouse_pos) ;
                    }
                    break ;

                case EventType::MouseUp:
                    if (e.GetMouseButton() == MouseButton::Left) {
                        Point mouse_pos = e.GetMousePos() ;
                        PointF mouse_posf = {static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)} ;
                        
                        vertical_slider.OnMouseUp(mouse_posf) ;
                        horizontal_slider.OnMouseUp(mouse_posf) ;
                        input.SetMouseUp(1) ; // Left button
                        input.SetMousePos(mouse_pos) ;
                    }
                    break ;

                case EventType::Slider : 
                    {
                        std::string event_type ;
                        switch (e.GetSliderEventType()) {
                            case SliderEventType::ValueChanged: event_type = "ValueChanged"; break ;
                            case SliderEventType::DragStart: event_type = "DragStart"; break ;
                            case SliderEventType::DragEnd: event_type = "DragEnd"; break ;
                            default: event_type = "Unknown"; break ;
                        }
                        
                        std::string slider_type = (e.GetSliderAddress() == &vertical_slider) ? "Vertical" : "Horizontal" ;
                        logger::info("Slider event - ", slider_type, " - Type: ", event_type, ", Value: ", e.GetSliderValue()) ;
                    }
                    break ;

                case EventType::KeyDown:
                    // Test programmatic value setting
                    if (e.GetKeyCode() == static_cast<uint32_t>(KeyCode::Space)) {
                        vertical_slider.SetValue(50.0f) ;
                        horizontal_slider.SetValue(75.0f) ;
                        logger::info("Reset sliders to default values") ;
                    }
                    break ;

                case EventType::Close:
                case EventType::Quit:
                    break ;

                default : break ;
            }
        }

        // Present both sliders
        vertical_slider.Present(window.GetHandle()) ;
        horizontal_slider.Present(window.GetHandle()) ;
        
        Sleep(16) ; // ~60 FPS
    }

    return 0 ;
}