#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;
	
	Window w1("zketch demo 8", 900, 800) ;
	w1.Show() ;

	Canvas c1 ;
	c1.Create({900, 800}) ;
	Drawer d ;

	if (d.Begin(c1)) {
		d.FillCircle({450, 400}, 200, rgba(255, 0, 0, 1)) ;
		d.DrawCircle({450, 400}, 200, rgba(255, 0, 0, 1), 10) ;
		d.End() ;
	}
	
	while(Application) {
		Event e ;
		while(PollEvent(e)) {
			switch (e) {
				case EventType::Quit :	
					logger::info("Quit") ;
			}
		}
		c1.Present(w1.GetHandle(), {100, 100}) ;
	}

	return 0 ;
}