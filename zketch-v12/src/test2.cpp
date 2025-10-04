#include "zketch.hpp"
using namespace zketch ;

// this code is tutorial for Event

struct Input {
	Event event_ ;
	InputSystem input_ ;
} ;

int main() {
	
	zketch_init() ;

	Window window("Event Demo", 500, 400) ;
	window.Show() ;

	Input in ;
	while (Application) {
		while (PollEvent(in.event_)) {
			switch (in.event_) {
				case EventType::Mouse :
					if (in.event_.GetMouseState() == MouseState::Up) {
						logger::info("Mouse Up [", DescMouseButton(in.event_.GetMouseButton()),"]\t- {", in.event_.GetMousePosition().x, ", ", in.event_.GetMousePosition().y, '}') ;
					} else if (in.event_.GetMouseState() == MouseState::Down) {
						logger::info("Mouse Down [", DescMouseButton(in.event_.GetMouseButton()),"]\t- {", in.event_.GetMousePosition().x, ", ", in.event_.GetMousePosition().y, '}') ;
					} else if (in.event_.GetMouseState() == MouseState::Wheel) {
						logger::info("Mouse Wheel\t- ", in.event_.GetMouseWheelValue()) ;
					} else if (in.event_.GetMouseState() == MouseState::None) {
					}
					break ;
				case EventType::Key :
					if (in.event_.GetKeyState() == KeyState::Up) {
						logger::info("Key Up\t\t- ", in.event_.GetKeyCode())	 ;
						in.input_.SetKeyUp(in.event_.GetKeyCode()) ;
					} else if (in.event_.GetKeyState() == KeyState::Down) {
						logger::info("Key Down\t- ", in.event_.GetKeyCode())	 ;
						in.input_.SetKeyDown(in.event_.GetKeyCode()) ;
					}
					break ;
				case EventType::Resize :
					logger::info("Resize\t\t- {", in.event_.GetResizedSize().x, ", ", in.event_.GetResizedSize().y, '}') ;
					break ;
			}

			if (in.input_.IsCtrlDown()) {
				if (in.event_.IsMouseEvent() && in.input_.IsMousePressed(MouseButton::Left)) {
					logger::info("CTRL + Mouse Up\t- {", in.event_.GetMousePosition().x, ", ", in.event_.GetMousePosition().y, '}') ;
				}
				in.input_.Update() ;
			} else {
				in.input_.Update() ;
			}

			Sleep(16) ;
		}
	}
}