#include <unordered_map>

#include "window.hpp"

using namespace zketch ;

static inline bool ISRUNNING = true ;

int main() {
	zketch::AppRegistry::RegisterWindowClass() ;

	std::unordered_map<HWND, std::unique_ptr<Window>> windows ;
	auto w1 = std::make_unique<Window>("Window 1", 300, 200) ;
	auto handle1 = w1->getHandle() ;
	windows[w1->getHandle()] = std::move(w1) ;
	auto w2 = std::make_unique<Window>("Window 2", 300, 200) ;
	auto handle2 = w2->getHandle() ;
	windows[w2->getHandle()] = std::move(w2) ;
	if (!handle1)
		logger::warning("handle is nullptr") ;
	windows[handle1]->Show() ;
	windows[handle2]->Show() ;

	while (ISRUNNING){
		Event e ;
		while (PollEvent(e)) {
			if (e == EventType::KeyDown) {
				if (e.keyCode() == 'Q') {
					windows[e.getHandle()]->Quit() ;
					windows.erase(e.getHandle()) ;
				}
				logger::info(char(e.keyCode()), " pressed from window : ", e.getHandle()) ;
			}
		}

		if (windows.empty()) {
			ISRUNNING = false ;
		}
	}
}