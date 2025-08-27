// Modern zketch demo - dramatically simplified usage
#include "zketch.hpp"

using namespace zketch;

class MyApp : public Window {
    float button_alpha = 1.0f;
    Point button_pos = {100, 100};
    bool button_hovered = false;
    
public:
    MyApp() : Window("Modern zketch Demo", {1280, 720}) {}
    
protected:
    void on_paint() override {
        auto& g = draw();
        
        // Background
        g.clear(colors::rgb(30, 30, 35));
        
        // Animated button
        Color btn_color = button_hovered ? 
            colors::rgba(100, 150, 255, int(button_alpha * 255)) :
            colors::rgba(80, 120, 200, int(button_alpha * 255));
            
        g.fill_rounded_rect({button_pos.x, button_pos.y, 200, 60}, 10, btn_color);
        g.draw_rounded_rect({button_pos.x, button_pos.y, 200, 60}, 10, Color::White(), 2);
        
        // Text
        g.draw_text(L"Click Me!", {button_pos.x + 70, button_pos.y + 20}, 
                   font("Arial", 16, FontStyle::Bold), Color::White());
        
        // Instructions
        g.draw_text(L"Press SPACE for animation, ESC to exit", {20, 20}, 
                   font("Arial", 14), colors::rgb(150, 150, 150));
    }
    
    void on_update(float dt) override {
        // Check hover
        Point mouse = input().mouse_pos();
        Rect button_rect = {button_pos.x, button_pos.y, 200, 60};
        button_hovered = button_rect.contains(mouse);
        
        // Handle input
        if (input().key_pressed(VK_SPACE)) {
            // Animate button position and alpha
            Point target = {float(rand() % 800), float(rand() % 400)};
            animate_to(animate(), button_pos, target, 0.8f, Easing::BounceOut);
            animate_to(animate(), button_alpha, 0.3f, 0.4f, Easing::QuadOut);
        }
        
        if (input().key_pressed(VK_ESCAPE)) {
            close();
        }
        
        // Mouse click animation
        if (input().mouse_pressed(0) && button_hovered) {
            animate_to(animate(), button_alpha, 0.1f, 0.2f, Easing::QuadOut);
        }
        
        // Reset alpha when not interacting
        if (!button_hovered && button_alpha < 1.0f) {
            animate_to(animate(), button_alpha, 1.0f, 0.3f, Easing::QuadOut);
        }
    }
    
    void on_resize(const Point& new_size) override {
        draw().begin(new_size); // Recreate graphics context
    }
};

// Ultra-simple main function
int main() {
    MyApp app;
    app.show();
    
    while (Window::process_messages() && !app.should_close()) {
        app.update(); // Handles timing, input, animation, and rendering
        Sleep(16); // ~60 FPS
    }
    
    return 0;
}