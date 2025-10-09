#include "zketch.hpp"
using namespace zketch ;

Rect Client ;

void UpdateClient(const Window& window) noexcept {
	Client = window.GetClientBound() ;
}

int main() {
	zketch_init() ;

	Window window("canvas renew test", 800, 600) ;
	window.Show() ;

	Renderer renderer ;

	Canvas canvas ;
	canvas.Create(50) ;

	if (renderer.Begin(canvas)) {
		renderer.Clear(Transparent) ;
		renderer.FillRect({Point{}, Size{50}}, Red) ;
		renderer.End() ;
	}

	PointF pos ;

	Event e ;
	while (Application::IsRunning()) {
		while (PollEvent(e)) {
			if (e == EventType::Close) {
				window.Close() ;
			}

			if (e == EventType::Key) {
				if (e.GetKeyState() == KeyState::Down) {
					if (e.GetKeyCode() == 'A') {
						pos += PointF{-10, 0} ;
					} else if (e.GetKeyCode() == 'W') {
						pos += PointF{0, -10} ;
					} else if (e.GetKeyCode() == 'D') {
						pos += PointF{10, 0} ;
					} else if (e.GetKeyCode() == 'S') {
						pos += PointF{0, 10} ;
					} 
				}
			}
		}

		if (renderer.Begin(window)) {
			renderer.Clear(White) ;
			renderer.DrawCanvas(&canvas, pos) ;
			renderer.End() ;
		}

		window.Present() ;
		Sleep(16) ;
	}

	return 0 ;
}