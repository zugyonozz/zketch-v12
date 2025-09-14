#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;

	Window w1("Window 1", 600, 400) ;
	Window w2("Window 2", 600, 400) ;
	Window w3(std::move(w1)) ;

	// w1.Show() ;
	w2.Show() ;
	w3.Show() ;

	while(Application) {
		Event ev ;
		while (PollEvent(ev)) {
			switch (ev) {
				case EventType::KeyDown : 
					logger::info("Key down : ", ev.keyCode()) ;
					break ;
				case EventType::KeyUp : 
					logger::info("Key up : ", ev.keyCode()) ;
					break ;
				case EventType::MouseUp : {
					auto pos = ev.mousePos() ;
					logger::info("Key down : {", pos.x, ", ", pos.y, '}') ;
					break ;
				}
				case EventType::MouseDown : 
					break ;
				default : break ;
			}
		}
	}

	return 0 ;
}