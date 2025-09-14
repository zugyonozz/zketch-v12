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
	Point p1 = {1, 2}, p2 = {2, 1} ;
	Point p3 = p1 + p2 ;
	
	while(Application) {
		Event e ;
		while(PollEvent(e)) {
			switch (e) {
				case EventType::Quit :	
					logger::info("Quit") ;
			}
		}
		c1.Present(w1.getHandle()) ;
	}

	return 0 ;
}