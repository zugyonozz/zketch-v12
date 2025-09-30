#include "builtin.hpp"
using namespace zketch ;

RectF RectClient ;

void UpdateClientRect(const Window& window) noexcept {
	std::cout << RectClient << '\n' ;
	RectClient = window.GetClientBound() ;
}

void UpdateBg(Canvas* canvas, const Color& color) noexcept {
	canvas->Create(RectClient.getSize()) ;
	Drawer draw ;
	if (!draw.Begin(*canvas)) {
		return ;
	}

	draw.Clear(rgba(0, 0, 0, 1)) ;
	draw.End() ;
}

int main() {
    // Initialize systems
    AppRegistry::RegisterWindowClass() ;
    EventSystem::Initialize() ;

    // Create main window
    Window window("Built-in Widgets Demo - zketch", 800, 600) ;
    window.Show() ;

	Canvas main ;
	main.Create(RectClient.getSize()) ;

	UpdateClientRect(window) ;

	Slider HSlider(
		Slider::Horizontal, 
		{0, RectClient.y, 10, RectClient.h},
		{15, 15}
	) ;

	HSlider.SetDrawer(
		[&](Canvas* canvas){
			Drawer draw ;
			if (HSlider.GetTrackBound().h != RectClient.h) {
				canvas->Create({5, RectClient.h}) ;
			}

			if (!draw.Begin(*canvas)) {
				return ;
			}

			draw.Clear(rgba(0, 0, 0, 1)) ;

			draw.FillRectRounded(HSlider.GetTrackBound(), rgba(60, 60, 60, 1), HSlider.GetTrackBound().h / 2) ;
			draw.FillRect(HSlider.GetThumbBound(), rgba(200, 200, 200, 1)) ;

			draw.End() ;
		}
	) ;

	while(Application) {

		Event e ;
		while(PollEvent(e)) {
			HSlider.OnHover(e) ;
			HSlider.OnPress(e) ;
			HSlider.OnDrag(e) ;
			HSlider.OnRelease(e) ;

			if (e.IsSliderEvent()) {
				if (e.GetSliderAddress() == &HSlider) {
					HSlider.Update() ;
				}
			}
			if (e.IsResizeEvent()) {
				UpdateClientRect(window) ;
				HSlider.Update() ;
				UpdateBg(&main, rgba(0, 0, 0, 1)) ;
				std::cout << HSlider.GetTrackBound() << '\n' ;
			}
		}

		main.Present(window.GetHandle()) ;
		HSlider.Present(window.GetHandle()) ;

		Sleep(16) ;
	}
}