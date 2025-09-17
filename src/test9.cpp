#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;
	AppRegistry::RegisterCommonControl() ;

	Window w1("Window 1", 600, 400) ;
	Rect r = w1.getClientBound() ;
	TrackBar t1(w1.getHandle(), {r.x + r.w - 30, r.y, 30, r.h}) ;

	Canvas c1 ;
	c1.Create({600, 400}) ; // Ukuran canvas sesuai window
	Drawer d ;

	w1.Show() ;

	bool needRedraw = true ; // Flag untuk mengontrol redraw

	while(Application) {
		Event ev ;
		bool hasEvents = false ;
		
		while (PollEvent(ev)) {
			hasEvents = true ;
			switch (ev) {
				case EventType::KeyDown : 
					logger::info("Key down : ", ev.keyCode()) ;
					// needRedraw = true ; // Mark untuk redraw jika ada input
					break ;
				case EventType::KeyUp : 
					logger::info("Key up : ", ev.keyCode()) ;
					break ;
				case EventType::MouseUp : {
					auto pos = ev.mousePos() ;
					logger::info("Mouse up : {", pos.x, ", ", pos.y, '}') ;
					// needRedraw = true ; // Mark untuk redraw
					break ;
				}
				case EventType::MouseDown : 
					// needRedraw = true ; // Mark untuk redraw
					break ;

				case EventType::TrackBar :
					logger::info("Scroll.\nType : ", ev.scrollBarType() == TrackBarType::HScroll ? "HSCROLL\n" : "VSCROLL\n", "Value : ", ev.scrollValue()) ;
					// needRedraw = true ; // Mark untuk redraw
					break ;

				case EventType::Resize : {
					auto newSize = ev.resizeSize() ;
					logger::info("Resize Event : {", newSize.x, ", ", newSize.y, '}') ;
					// Resize canvas sesuai ukuran baru window
					if (newSize.x > 0 && newSize.y > 0) {
						c1.Create({static_cast<int32_t>(newSize.x), static_cast<int32_t>(newSize.y)}) ;
					}
					needRedraw = true ;
					break ;
				}
				default : break ;
			}
		}

		if (needRedraw) {
			if (d.Begin(c1)) {
				d.Clear(rgba(0, 0, 0, 255)) ; 
				Rect clientRect = w1.getClientBound() ;
				Point center = {clientRect.w / 2, clientRect.h / 2} ;
				d.FillCircle(center, 50, rgba(255, 0, 0, 1)) ;
				d.DrawCircle(center, 50, rgba(255, 255, 255, 1), 3) ;
				d.DrawString( // render string
					L"ZKETCH", {center.x - 110, center.y + 100}, 
					rgba(255, 0, 0, 1), 
					{L"Arial", 48.0f, FontStyle::BoldItalic}) ;
				d.End() ;
			}
			needRedraw = false ;
		}

		c1.Present(w1.getHandle()) ;
		Sleep(16) ; // ~60 FPS
	}
	return 0 ;
}