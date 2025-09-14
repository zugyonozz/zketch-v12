#include "window.hpp"

int main() {

	zketch::AppRegistry::RegisterWindowClass() ;

	zketch::Window win("Test 4", 800, 600) ;
	win.Show() ;

	zketch::Event e = zketch::Event::createEvent() ;
	while(zketch::PollEvent(e)) {
		if (e == zketch::EventType::KeyDown) {
			if (e.keyCode() == 'Q')
				PostQuitMessage(0) ;
			else
			 	zketch::logger::info(char(e.keyCode()), " Pressed") ;
		}
	}

	return 0;
}