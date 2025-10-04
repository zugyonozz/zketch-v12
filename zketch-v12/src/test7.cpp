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
					in.SetKeyDown(e.GetKeyCode()) ;
					logger::info("KeyDown -> ", e.GetKeyCode()) ;
					break ;
				case EventType::KeyUp : 
					in.SetKeyUp(e.GetKeyCode()) ;
					logger::info("KeyUp -> ", e.GetKeyCode()) ;
					break ;
				case EventType::MouseDown :
					logger::info("MouseDown -> ", MouseButtonListener(e.GetMouseButton())) ;
				case EventType::MouseMove :
					logger::info(" -> ", e.GetMousePos().x, ", ", e.GetMousePos().y) ;
					break ;
				case EventType::MouseWheel :
					logger::info(" -> ", e.GetMouseDelta()) ;
				
			}
		}
		if (in.IsCtrlDown() && in.IsKeyDown(KeyCode::A)) {
			logger::info("CTRL + A Pressed") ;
		} else if (in.IsCtrlDown() && in.IsKeyDown(KeyCode::B)) {
			logger::info("CTRL + B Pressed") ;
		}

		in.Update() ;
		Sleep(16) ;
	}
}