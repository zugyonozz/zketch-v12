#include "window.hpp"
using namespace zketch ;

int main() {
	AppRegistry::RegisterWindowClass() ;

	Window window("Slider Demo", 900, 800) ;
	window.Show() ;

	RectF client = window.GetClientBound() ;

	Slider slider(Slider::Vertical, {client.w - 20, client.y, 20, client.h - 40}, 0, 100, 60) ;

	auto& style = slider.GetStyle();
    style.thumb_fill = Color(255, 100, 100, 255);      // Red thumb
    style.thumb_hover = Color(255, 150, 150, 255);     // Light red on hover
    style.track_fill = Color(100, 100, 255, 255);      // Blue track
    style.thumb_size = 20.0f;                          // Bigger thumb
    style.track_thickness = 8.0f;                      // Thicker track

	while (Application) {
		Event e ;
		while (PollEvent(e)) {
			switch (e) {
				case EventType::Slider : 
					logger::info("Slider event from EventSystem - Type: ", static_cast<int>(e.sliderEventType()), ", Value: ", e.sliderValue()) ;
					break ;
				default : break ;
			}
		}

		slider.Present(window.GetHandle()) ;
		Sleep(16) ;
	}
}