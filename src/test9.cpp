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

				case EventType::TrackBar :
					logger::info("Scroll.\nType : ", ev.scrollBarType() == TrackBarType::HScroll ? "HSCROLL\n" : "VSCROLL\n", "Value : ", ev.scrollValue()) ;

				case EventType::Resize :
					logger::info("Resize Event : {", ev.resizeSize().x, ", ", ev.resizeSize().y, '}') ;
					c1.MarkDirty() ;
				default : break ;
			}
		}
		if (d.Begin(c1)) {
			Rect r_ = w1.getClientBound() ;
			d.FillCircle({r_.w / 2 - 100, r_.h / 2 - 100}, 200, rgba(255, 0, 0, 1)) ;
			d.DrawCircle({r_.w / 2 - 100, r_.h / 2 - 100}, 200, rgba(255, 0, 0, 1), 10) ;
			d.End() ;
		}

		c1.Present(w1.getHandle()) ;
		Sleep(16) ;
	}

	return 0 ;
}