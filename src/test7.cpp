#include "window.hpp"

using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;
	InputSystem in ;

	Window w1 = {"zketch", 400, 300} ;
	w1.Show() ;

	while(Application) {
		Event e ;
		while(PollEvent(e)) {
			switch (e) {
				case EventType::KeyDown : 
					in.SetKeyDown(e.keyCode()) ;
					logger::info("KeyDown ", e.keyCode()) ;
					break ;
				case EventType::KeyUp : 
					in.SetKeyUp(e.keyCode()) ;
					logger::info("KeyUp ", e.keyCode()) ;
					break ;
				case EventType::MouseMove :
					logger::info("Mouse move ", e.mousePos().x, ", ", e.mousePos().y) ;
					break ;
				
			}
		}
		if (in.isCtrlDown() && in.isKeyDown(KeyCode::A)) {
			logger::info("CTRL + A Pressed") ;
		} else if (in.isCtrlDown() && in.isKeyDown(KeyCode::B)) {
			logger::info("CTRL + B Pressed") ;
		}
		in.Update() ;
		Sleep(16) ;
	}
}