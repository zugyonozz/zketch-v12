#include "zketch.hpp"

using namespace zketch ;

int main() {
    zketch_init() ;

	Renderer render ;
    
    Window window("Test Window", 800, 600) ;
	window.Show() ;

    Button button({100, 100, 200, 50}, Font("Arial", 20), L"Click Me") ;
    Slider slider(Slider::Horizontal, {100, 200, 300, 20}, {20, 30}) ;
    TextBox textbox({100, 300, 400, 100}, L"Hello World", Font("Arial", 20)) ;
	InputBox textinput({400, 100, 200, 50}, Font("Arial", 20), 500) ;
    
    button.SetCallback([]() {
        logger::info("Button clicked!") ;
    }) ;

	textinput.SetCallback([](){
		logger::info("textinput Submited!") ;
	}) ;
    
	Event e ;
    InputSystem input ;
    
    while (Application::IsRunning()) {
        while (PollEvent(e)) {
            if (e == EventType::Close) {
                window.Close() ;
            }
            
            if (e.IsMouseEvent()) {
                button.OnHover(e.GetMousePosition()) ;
                slider.OnHover(e.GetMousePosition()) ;
				textinput.OnHover(e.GetMousePosition()) ;
                
                if (e.GetMouseState() == MouseState::Down) {
                    button.OnPress(e.GetMousePosition()) ;
                    slider.OnPress(e.GetMousePosition()) ;
					textinput.OnPress(e.GetMousePosition()) ;
					if (textinput.OnRelease(e.GetMousePosition()) && textinput.IsActive()) {
						textinput.Deactivate() ;
					}
                }
                
                if (e.GetMouseState() == MouseState::Up) {
                    button.OnRelease(e.GetMousePosition()) ;
                    slider.OnRelease() ;
                }
                
                slider.OnDrag(e.GetMousePosition()) ;
            }

			if (e == EventType::Key) {
				if (e.GetKeyState() == KeyState::Down) {
					if (textinput.IsActive()) {
						if ((e.GetKeyCode() >= '0' && e.GetKeyCode() <= 'Z') || e.GetKeyCode() == ' ') {
							textinput.Insert(static_cast<wchar_t>(e.GetKeyCode())) ;
						} else if (e.GetKeyCode() == static_cast<uint32_t>(KeyCode::ArrowLeft)) {
							textinput.MoveCursorPrev() ;
						} else if (e.GetKeyCode() == static_cast<uint32_t>(KeyCode::ArrowRight)) {
							textinput.MoveCursorNext() ;
						} else if (e.GetKeyCode() == static_cast<uint32_t>(KeyCode::Enter)) {
							textinput.Submit() ;
						} else if (e.GetKeyCode() == static_cast<uint32_t>(KeyCode::Backspace)) {
							textinput.Backspace() ;
						}
					}
				}
			}
        }
        
		textinput.UpdateCursor() ;
		button.InvokeUpdate() ;
		slider.InvokeUpdate() ;
		textbox.InvokeUpdate() ;
		textinput.InvokeUpdate() ;

        if (render.Begin(window)) {
			render.Clear(White) ;
			render.DrawCanvas(button.GetCanvas(), button.GetPosition()) ;
			render.DrawCanvas(slider.GetCanvas(), slider.GetPosition()) ;
			render.DrawCanvas(textbox.GetCanvas(), textbox.GetPosition()) ;
			render.DrawCanvas(textinput.GetCanvas(), textinput.GetPosition()) ;
			render.End() ;
		}

		window.Present() ;
        input.Update() ;
    }
    
    return 0;
}