#include "slider.hpp"
using namespace zketch ;

int main() {

	AppRegistry::RegisterWindowClass() ;

	Window window("Re-Model Event Structure", 900, 800) ;
	window.Show() ;

	Canvas canvas ;
	Drawer drawer ;

	bool NeedRedraw = true ;
	Event e ;

	while (Application) {
		while (PollEvent(e)) {
			logger::info(EventDescribe(e)) ;
			if (e.IsResizeEvent()) {
				NeedRedraw = true ;
			}
		} 
		if (NeedRedraw) {
			canvas.Create(e.GetResizedSize()) ;
			if (drawer.Begin(canvas)) {
				drawer.Clear(rgba(255, 0, 0, 1)) ;
				drawer.End() ;
			}
			NeedRedraw = false ;
		}
		canvas.Present(window.GetHandle()) ;
		Sleep(16) ;
	}
}