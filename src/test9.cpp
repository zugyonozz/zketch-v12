#include "window.hpp"
using namespace zketch ;

void SliderEventListener(SliderEvent event) noexcept {
	
}

int main() {
	AppRegistry::RegisterWindowClass() ;

	Window window("Slider Demo", 900, 700) ;
	window.Show() ;

	Rect clientbound = window.GetClientBound() ;

	Slider slider(Slider::Vertical, clientbound) ;
}