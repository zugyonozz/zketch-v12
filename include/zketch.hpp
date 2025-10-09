#pragma once
#include "renderer.hpp"
#include "inputsystem.hpp"
#include "slider.hpp"
#include "button.hpp"
#include "textbox.hpp"
#include "textinput.hpp"

namespace zketch {
	void zketch_init() noexcept {

		#ifdef INIT_DEBUG
			auto tm0 = std::chrono::high_resolution_clock::now() ;
		#endif

		std::thread t1(Application::LoadFonts) ;
		AppRegistry::RegisterWindowClass() ;
		EventSystem::Init() ;
		t1.join() ;

		#ifdef INIT_DEBUG
			auto tm1 = std::chrono::high_resolution_clock::now() ;
			logger::info("Init time : ", std::chrono::duration_cast<std::chrono::milliseconds>(tm1 - tm0).count(), " ms.") ;
		#endif
	}
}