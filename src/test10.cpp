#include "builtin.hpp"
using namespace zketch ;

Rect Client ;

void InvokeRedrawHSlider(Canvas* canvas, const Slider& slider) noexcept {
	Drawer draw ;
	
	if (!draw.Begin(*canvas)) {
		return ;
	}

	draw.Clear(rgba(60, 60, 60, 1)) ;
	if (slider.IsHovered()) {
		draw.FillRectRounded(slider.GetRelativeThumbBound(), rgba(200, 100, 100, 1), slider.GetRelativeThumbBound().w / 2) ;
	} else {
		draw.FillRectRounded(slider.GetRelativeThumbBound(), rgba(200, 200, 200, 1), slider.GetRelativeThumbBound().w / 2) ;
	}
	draw.End() ;
}

int main() {
    AppRegistry::RegisterWindowClass() ;
    EventSystem::Initialize() ;

    Window window("Built-in Demo", 800, 600) ;
    window.Show() ;

	Client = window.GetClientBound() ;

	Slider HSlider(Slider::Vertical, {Client.w - 10, Client.y, 10, Client.h}, {10, 60}) ;

	HSlider.SetDrawer(InvokeRedrawHSlider) ;

	Event e ;

    while (Application) {
		while (PollEvent(e)) {
			if (e == EventType::Mouse) {
				if (e.GetMouseState() == MouseState::Up) {
					HSlider.OnPress(e.GetMousePosition()) ;
				} else if (e.GetMouseState() == MouseState::Down) {
					HSlider.OnRelease() ;
				} else if (e.GetMouseState() == MouseState::Wheel) {

				} else if (e.GetMouseState() == MouseState::None) {
					HSlider.OnHover(e.GetMousePosition()) ;
				}
			}

			if (e == EventType::Slider) {
				if (e.GetSliderState() == SliderState::Start) {
					logger::info("Slider::Start") ;
				} else if (e.GetSliderState() == SliderState::Changed) {
					logger::info("Slider::Changed -> ", e.GetSliderValue()) ;
				} else if (e.GetSliderState() == SliderState::End) {
					logger::info("Slider::End") ;
				} else if (e.GetSliderState() == SliderState::Hover) {
					logger::info("Slider::Hover") ;
				}
			}

			if (e == EventType::Resize) {
				HSlider.Update() ;
			}
		}

		HSlider.Present(window.GetHandle()) ;
		Sleep(16) ;
	}
}