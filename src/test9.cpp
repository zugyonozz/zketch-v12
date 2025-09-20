#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;

	Window w1("Window 1", 600, 400) ;
	Rect r = w1.GetClientBound() ;
	Point s = r.getSize() ;
	Slider slider(Slider::Vertical, {r.x + r.w - 20, r.y, 20, r.h}, {20, 20}) ;

	Canvas c1 ;
	c1.Create({s.x - 20, s.y}) ; // Ukuran canvas sesuai window
	Drawer d ;

	w1.Show() ;

	bool needRedraw = true ; // Flag untuk mengontrol redraw
	InputSystem in ;

	while(Application) {
		Event ev ;
		bool hasEvents = false ;
		
		while (PollEvent(ev)) {
			hasEvents = true ;
			switch (ev) {
				case EventType::KeyDown : 
					logger::info("Key down : ", ev.keyCode()) ;
					// needRedraw = true ; // Mark untuk redraw jika ada input
					in.SetKeyDown(ev.keyCode()) ;
					break ;
				case EventType::KeyUp : 
					logger::info("Key up : ", ev.keyCode()) ;
					in.SetKeyUp(ev.keyCode()) ;
					break ;
				case EventType::MouseUp : {
					auto pos = ev.mousePos() ;
					logger::info("Mouse up : {", pos.x, ", ", pos.y, '}') ;
					if (slider.OnDrag()) {
						slider.OnMouseUp() ;
					}
					break ;
				}
				case EventType::MouseMove :
					if (slider.OnDrag()) {
						slider.OnMouseMove(ev.mousePos()) ;
					}
					break ;
				case EventType::MouseDown : 
					slider.OnMouseDown(ev.mousePos()) ;
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
						c1.Create({static_cast<int32_t>(newSize.x - 20), static_cast<int32_t>(newSize.y)}) ;
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
				r = w1.GetClientBound() ;
				Point center = {r.w / 2, r.h / 2} ;
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

		if (in.isCtrlDown() && in.isKeyDown(KeyCode::A)) {
			logger::info("CTRL + A pressed") ;
		}
		slider.Present(w1.GetHandle()) ;
		Sleep(16) ; // ~60 FPS
	}
	return 0 ;
}