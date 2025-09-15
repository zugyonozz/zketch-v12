#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;
	AppRegistry::RegisterCommonControl() ;

	Window w1("Window 1", 600, 400) ;
	Window w2("Window 2", 600, 400) ;
	Rect r = w1.getClientBound() ;
	TrackBar t1(w1.getHandle(), {r.x + r.w - 30, r.y, 30, r.h}) ;

	Canvas c1 ;
	c1.Create({900, 800}) ;
	Drawer d ;

	if (d.Begin(c1)) {
		d.FillCircle({450, 400}, 200, rgba(255, 0, 0, 1)) ;
		d.DrawCircle({450, 400}, 200, rgba(255, 0, 0, 1), 10) ;
		d.End() ;
	}

	w1.Show() ;
	w2.Show() ;

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
				case EventType::ScrollBar :
					logger::info("Scroll.\nType : ", ev.scrollBarType() == ScrollBarType::HScroll ? "HSCROLL\n" : "VSCROLL\n", "Value : ", ev.scrollValue()) ;
				case  EventType::Resize :
					logger::info("Resize Event : {", ev.resizeSize().x, ", ", ev.resizeSize().y, '}') ;
				default : break ;
			}
		}
		c1.Present(w1.getHandle()) ;
	}

	return 0 ;
}