#include "zketch.hpp"
#include <chrono>
#include <sstream>

using namespace zketch;
struct init__ {
	init__() {
		zketch_init() ;
	}
} init_ ;

// Performance monitoring utilities
class PerformanceMonitor {
private:
    std::chrono::high_resolution_clock::time_point start_;
    std::string label_;
    
public:
    PerformanceMonitor(const std::string& label) : label_(label) {
        start_ = std::chrono::high_resolution_clock::now();
    }
    
    ~PerformanceMonitor() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        logger::info(label_, " took ", duration / 1000.0, "ms");
    }
};

#define PROFILE_SCOPE(name) PerformanceMonitor _perf_##__LINE__(name)

// Example 1: Basic widget usage with optimizations
void Example_BasicWidgets() {
    Window window("Optimized Widgets Example", 800, 600);
    window.Show();
    
    // Configure bitmap pool
    g_bitmap_pool.SetMaxMemory(128 * 1024 * 1024); // 128MB
    
    // Create widgets
    Button button1(RectF{50, 50, 200, 50}, L"Click Me", Font(L"Arial", 14, FontStyle::Bold));
    Button button2(RectF{50, 120, 200, 50}, L"Disabled", Font(L"Arial", 14, FontStyle::Regular));
    button2.SetEnabled(false);
    
    Slider slider(Slider::Horizontal, RectF{50, 200, 300, 20}, SizeF{20, 30});
    slider.SetRange(0, 100);
    slider.SetValue(50);
    
    TextBox textbox(RectF{50, 250, 400, 100}, L"Hello, World!", Font(L"Arial", 12, FontStyle::Regular));
    
    // Stats display
    TextBox stats(RectF{50, 400, 500, 150}, L"", Font(L"Courier New", 10, FontStyle::Regular));
    stats.SetBackgroundColor(rgba(240, 240, 240, 255));
    
    // Event handlers
    button1.SetOnClick([&]() {
        logger::info("Button 1 clicked!");
        textbox.AppendText(L"\nButton clicked!");
    });
    
    InputSystem input;
    Event event;
    
    uint32_t frame_count = 0;
    auto last_fps_update = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    
    while (Application) {
        input.Update();
        
        while (PollEvent(event)) {
            if (event == EventType::Close) {
                Application.QuitProgram();
            }
            
            if (event.IsKeyEvent()) {
                if (event.GetKeyState() == KeyState::Down) {
                    input.SetKeyDown(event.GetKeyCode());
                } else {
                    input.SetKeyUp(event.GetKeyCode());
                }
            }
            
            if (event.IsMouseEvent()) {
                Point mouse_pos = event.GetMousePosition();
                PointF mouse_posf{static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)};
                
                input.SetMousePos(mouse_pos);
                
                if (event.GetMouseState() == MouseState::None) {
                    button1.OnHover(mouse_posf);
                    button2.OnHover(mouse_posf);
                    slider.OnHover(mouse_posf);
                    
                    if (slider.IsDragging()) {
                        slider.OnDrag(mouse_posf);
                    }
                }
                
                if (event.GetMouseState() == MouseState::Down) {
                    if (event.GetMouseButton() == MouseButton::Left) {
                        button1.OnPress(mouse_posf);
                        button2.OnPress(mouse_posf);
                        slider.OnPress(mouse_posf);
                        input.SetMouseDown(MouseButton::Left);
                    }
                }
                
                if (event.GetMouseState() == MouseState::Up) {
                    if (event.GetMouseButton() == MouseButton::Left) {
                        button1.OnRelease(mouse_posf);
                        button2.OnRelease(mouse_posf);
                        slider.OnRelease();
                        input.SetMouseUp(MouseButton::Left);
                    }
                }
            }
        }
        
        // Update stats
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update).count();
        
        if (elapsed >= 500) { // Update every 500ms
            fps = (frame_count * 1000.0) / elapsed;
            frame_count = 0;
            last_fps_update = now;
            
            std::wostringstream oss;
            oss << L"=== Performance Stats ===\n";
            oss << L"FPS: " << static_cast<int>(fps) << L"\n";
            oss << L"Slider Value: " << static_cast<int>(slider.GetValue()) << L"\n";
            oss << L"Pool Memory: " << (g_bitmap_pool.GetMemoryUsage() / (1024*1024)) << L"MB\n";
            oss << L"Pool Bitmaps: " << g_bitmap_pool.GetBitmapCount() << L"\n";
            oss << L"\nWidget States:\n";
            oss << L"Button1: " << (button1.IsHovered() ? L"Hovered " : L"") 
                << (button1.IsPressed() ? L"Pressed" : L"") << L"\n";
            oss << L"Slider: " << (slider.IsHovered() ? L"Hovered " : L"")
                << (slider.IsDragging() ? L"Dragging" : L"") << L"\n";
            
            stats.SetText(oss.str());
        }
        
        // Render
        {
            PROFILE_SCOPE("Frame Render");
            
            button1.Present(window);
            button2.Present(window);
            slider.Present(window);
            textbox.Present(window);
            stats.Present(window);
        }
        
        frame_count++;
        
        // Cap at 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// Example 2: Stress test - many widgets
void Example_StressTest() {
    Window window("Stress Test - 100 Widgets", 1280, 720);
    window.Show();
    
    g_bitmap_pool.SetMaxMemory(256 * 1024 * 1024); // 256MB
    
    constexpr int GRID_COLS = 10;
    constexpr int GRID_ROWS = 10;
    constexpr float WIDGET_SIZE = 80.0f;
    constexpr float PADDING = 10.0f;
    
    std::vector<std::unique_ptr<Button>> buttons;
    
    // Create 100 buttons
    logger::info("Creating 100 buttons...");
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            float x = 10 + col * (WIDGET_SIZE + PADDING);
            float y = 10 + row * (WIDGET_SIZE + PADDING);
            
            auto button = std::make_unique<Button>(
                RectF{x, y, WIDGET_SIZE, WIDGET_SIZE},
                std::to_wstring(row * GRID_COLS + col),
                Font(L"Arial", 12, FontStyle::Regular)
            );
            
            // Varied colors
            int idx = row * GRID_COLS + col;
            button->SetDrawer([idx](Renderer* renderer, const Button& btn) {
                uint8_t r = (idx * 37) % 256;
                uint8_t g = (idx * 73) % 256;
                uint8_t b = (idx * 131) % 256;
                
                Color base_color = rgba(r, g, b, 255);
                Color hover_color = rgba(
                    std::min(r + 30, 255),
                    std::min(g + 30, 255),
                    std::min(b + 30, 255),
                    255
                );
                
                RectF rect = btn.GetRelativeBound();
                renderer->FillRectRounded(
                    rect,
                    btn.IsHovered() ? hover_color : base_color,
                    5.0f
                );
                
                renderer->DrawString(
                    btn.GetLabel(),
                    {static_cast<int32_t>(rect.w / 2 - 10), 
                     static_cast<int32_t>(rect.h / 2 - 10)},
                    rgba(255, 255, 255, 255),
                    btn.GetFont()
                );
            });
            
            buttons.push_back(std::move(button));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto creation_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    logger::info("Created 100 buttons in ", creation_time, "ms");
    logger::info("Pool memory: ", g_bitmap_pool.GetMemoryUsage() / (1024*1024), "MB");
    logger::info("Pool bitmaps: ", g_bitmap_pool.GetBitmapCount());
    
    InputSystem input;
    Event event;
    
    uint32_t frame_count = 0;
    auto last_fps_update = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    double avg_render_time = 0.0;
    
    while (Application) {
        input.Update();
        
        while (PollEvent(event)) {
            if (event == EventType::Close) {
                Application.QuitProgram();
            }
            
            if (event.IsMouseEvent()) {
                Point mouse_pos = event.GetMousePosition();
                PointF mouse_posf{static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)};
                
                input.SetMousePos(mouse_pos);
                
                // Update all buttons
                if (event.GetMouseState() == MouseState::None) {
                    for (auto& btn : buttons) {
                        btn->OnHover(mouse_posf);
                    }
                }
                
                if (event.GetMouseState() == MouseState::Down &&
                    event.GetMouseButton() == MouseButton::Left) {
                    for (auto& btn : buttons) {
                        btn->OnPress(mouse_posf);
                    }
                }
                
                if (event.GetMouseState() == MouseState::Up &&
                    event.GetMouseButton() == MouseButton::Left) {
                    for (auto& btn : buttons) {
                        btn->OnRelease(mouse_posf);
                    }
                }
            }
        }
        
        // Render all buttons
        auto render_start = std::chrono::high_resolution_clock::now();
        
        for (auto& btn : buttons) {
            btn->Present(window);
        }
        
        auto render_end = std::chrono::high_resolution_clock::now();
        auto render_time = std::chrono::duration_cast<std::chrono::microseconds>(
            render_end - render_start
        ).count() / 1000.0;
        
        avg_render_time = (avg_render_time * 0.9) + (render_time * 0.1); // Moving average
        
        // Update FPS
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update).count();
        
        if (elapsed >= 1000) {
            fps = (frame_count * 1000.0) / elapsed;
            frame_count = 0;
            last_fps_update = now;
            
            logger::info("FPS: ", static_cast<int>(fps),
                        ", Avg Render: ", avg_render_time, "ms",
                        ", Memory: ", g_bitmap_pool.GetMemoryUsage() / (1024*1024), "MB");
        }
        
        frame_count++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// Example 3: Dynamic widget creation/destruction
void Example_DynamicWidgets() {
    Window window("Dynamic Widgets Test", 800, 600);
    window.Show();
    
    g_bitmap_pool.SetMaxMemory(128 * 1024 * 1024);
    
    std::vector<std::unique_ptr<Button>> buttons;
    
    Button add_button(RectF{10, 10, 150, 40}, L"Add Widget", Font(L"Arial", 12, FontStyle::Regular));
    Button remove_button(RectF{170, 10, 150, 40}, L"Remove Widget", Font(L"Arial", 12, FontStyle::Regular));
    Button clear_button(RectF{330, 10, 150, 40}, L"Clear All", Font(L"Arial", 12, FontStyle::Regular));
    
    TextBox info(RectF{10, 60, 400, 80}, L"", Font(L"Arial", 10, FontStyle::Regular));
    
    add_button.SetOnClick([&]() {
        float x = 10 + (buttons.size() % 7) * 110;
        float y = 150 + (buttons.size() / 7) * 60;
        
        auto btn = std::make_unique<Button>(
            RectF{x, y, 100, 50},
            L"#" + std::to_wstring(buttons.size()),
            Font(L"Arial", 10, FontStyle::Regular)
        );
        
        buttons.push_back(std::move(btn));
        
        std::wostringstream oss;
        oss << L"Widgets: " << buttons.size() << L"\n";
        oss << L"Pool Memory: " << (g_bitmap_pool.GetMemoryUsage() / (1024*1024)) << L"MB\n";
        oss << L"Pool Bitmaps: " << g_bitmap_pool.GetBitmapCount();
        info.SetText(oss.str());
    });
    
    remove_button.SetOnClick([&]() {
        if (!buttons.empty()) {
            buttons.pop_back();
            
            std::wostringstream oss;
            oss << L"Widgets: " << buttons.size() << L"\n";
            oss << L"Pool Memory: " << (g_bitmap_pool.GetMemoryUsage() / (1024*1024)) << L"MB\n";
            oss << L"Pool Bitmaps: " << g_bitmap_pool.GetBitmapCount();
            info.SetText(oss.str());
        }
    });
    
    clear_button.SetOnClick([&]() {
        buttons.clear();
        
        std::wostringstream oss;
        oss << L"Widgets cleared!\n";
        oss << L"Pool Memory: " << (g_bitmap_pool.GetMemoryUsage() / (1024*1024)) << L"MB\n";
        oss << L"Pool Bitmaps: " << g_bitmap_pool.GetBitmapCount() << L"\n";
        oss << L"Note: Memory will be reused";
        info.SetText(oss.str());
    });
    
    InputSystem input;
    Event event;
    
    while (Application) {
        input.Update();
        
        while (PollEvent(event)) {
            if (event == EventType::Close) {
                Application.QuitProgram();
            }
            
            if (event.IsMouseEvent()) {
                Point mouse_pos = event.GetMousePosition();
                PointF mouse_posf{static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)};
                
                add_button.OnHover(mouse_posf);
                remove_button.OnHover(mouse_posf);
                clear_button.OnHover(mouse_posf);
                
                for (auto& btn : buttons) {
                    btn->OnHover(mouse_posf);
                }
                
                if (event.GetMouseState() == MouseState::Down &&
                    event.GetMouseButton() == MouseButton::Left) {
                    add_button.OnPress(mouse_posf);
                    remove_button.OnPress(mouse_posf);
                    clear_button.OnPress(mouse_posf);
                    
                    for (auto& btn : buttons) {
                        btn->OnPress(mouse_posf);
                    }
                }
                
                if (event.GetMouseState() == MouseState::Up &&
                    event.GetMouseButton() == MouseButton::Left) {
                    add_button.OnRelease(mouse_posf);
                    remove_button.OnRelease(mouse_posf);
                    clear_button.OnRelease(mouse_posf);
                    
                    for (auto& btn : buttons) {
                        btn->OnRelease(mouse_posf);
                    }
                }
            }
        }
        
        // Render
        add_button.Present(window);
        remove_button.Present(window);
        clear_button.Present(window);
        info.Present(window);
        
        for (auto& btn : buttons) {
            btn->Present(window);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main() {    
    // Choose example to run
    logger::info("=== Optimized Rendering System Examples ===");
    logger::info("1. Basic Widgets Example");
    logger::info("2. Stress Test (100 widgets)");
    logger::info("3. Dynamic Widget Creation");
    
    int choice = 2; // Change this to run different examples
    
    switch (choice) {
        case 1:
            Example_BasicWidgets();
            break;
        case 2:
            Example_StressTest();
            break;
        case 3:
            Example_DynamicWidgets();
            break;
        default:
            logger::error("Invalid choice");
            break;
    }
    
    // Cleanup
    g_bitmap_pool.Clear();
    
    return 0;
}