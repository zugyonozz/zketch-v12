#include "zketch.hpp"
using namespace zketch ;

// this code is tutorial create window

int main() {
	zketch_init() ;

	// create window with ctor with params
	// paramater :
	// - "title"
	// - "width"
	// - "height"
	Window window("Window Demo", 900, 800) ;

	// this method using for showing window
	window.Show() ;

	// Create Event buffer
	Event e ;

	while(Application) {

		// push event to event queue
		while(PollEvent(e)) {

		}
		
		// delay 16 ms equal 60 FPS
		Sleep(16) ;
	}
}