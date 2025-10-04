#pragma once
#include "window.hpp"
#include "inputsystem.hpp"
#include "slider.hpp"
#include "button.hpp"
#include "textbox.hpp"

namespace zketch {
	void zketch_init() noexcept {
		AppRegistry::RegisterWindowClass() ;
		EventSystem::Initialize() ;
	}
}