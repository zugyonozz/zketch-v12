#include "window.hpp"

using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;
	
	auto w1 = std::make_unique<Window>("Window 1", 100, 100, 500, 400) ;
	
	auto w2 = std::make_unique<Window>("Window 2", 650, 100, 500, 400) ;

	w1->Show() ;
	w2->Show() ;
	
	while (Application) {
		Event e ;
		while (PollEvent(e)) {
			switch (e) {
				case EventType::Close:
					break ;
					
				case EventType::Resize: {
					Size size = e.resizeSize() ;
					logger::info("Resize Event, size : ", size.x, ", ", size.y) ;
					break ;
				}
				
				case EventType::Quit :
					break ;
					
				case EventType::KeyDown :
					if (e.keyCode() == '\33')
						Application.QuitWindow(e.getHandle()) ;
					break ;
					
				case EventType::KeyUp :
					logger::info("KeyUp : ", e.keyCode()) ;
					break ;
					
				case EventType::MouseDown : 
					break ;
				
				case EventType::MouseUp : 
					logger::info("MouseUp : ", e.mousePos().x, ", ", e.mousePos().y) ;
					break ;
				
				case EventType::MouseMove : 
					break ;
				
				case EventType::None :
					break ;
					
				default:
					break ;
			}
		}
		Sleep(1000) ;
	}
	return 0 ;
}