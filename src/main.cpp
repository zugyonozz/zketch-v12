#include "window_.h"

void Handler(const zketch::Event& e) {
	if (e == zketch::EventType::Quit)
		zketch::IS_RUNNING = false ;
	if (e == zketch::EventType::KeyDown)
		if (e.keyCode() == 'A')
			zketch::logger::info("A Pressed!\n") ;
}

int main() {
	zketch::Application app ;
	HWND w1 = app.createNewWindow("Demo", 900, 800) ;
	app.AddHandler(w1, Handler) ;
	return app.Run() ;

	return 0 ;
}