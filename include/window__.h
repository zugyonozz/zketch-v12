#pragma once

#include <memory>
#include <unordered_map>

#include "event_.h"
#include "texture.h"

namespace zketch {

	struct __App_Registry__ {
		static inline const char* g_window_class_name_ = "zketch_app" ;
		static inline bool g_was_registered_ = false ;
	} ;

	void SetWindowClass(const char* classname) noexcept {
		logger::info("Changing window class name.") ;
		if (!__App_Registry__::g_was_registered_)
			__App_Registry__::g_window_class_name_ = classname ;
	}
	
	static inline HINSTANCE g_hinstance_ = GetModuleHandle(nullptr) ;

	static LRESULT CALLBACK g_window_procedure_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

	bool RegisterWindowClass() {
		logger::info("Registering window class...") ;

		WNDCLASSEX wc {} ;

		wc.cbSize = sizeof(WNDCLASSEXA) ;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
		wc.lpfnWndProc = g_window_procedure_ ;
		wc.cbClsExtra = 0 ;
		wc.cbWndExtra = 0 ;
		wc.hInstance = g_hinstance_ ;
		wc.hIcon = LoadIconA(nullptr, IDI_APPLICATION) ;
		wc.hCursor = LoadCursorA(nullptr, IDC_ARROW) ;
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1) ;
		wc.lpszMenuName = nullptr ;
		wc.lpszClassName = __App_Registry__::g_window_class_name_ ;
		wc.hIconSm = LoadIconA(nullptr, IDI_APPLICATION) ;

		ATOM result = RegisterClassEx(&wc) ;
		if (result == 0) {
			DWORD error = GetLastError() ;
			if (error == ERROR_CLASS_ALREADY_EXISTS) {
				logger::info("Window class registered successfully.") ;
				__App_Registry__::g_was_registered_ = true ;
				return true ;
			} else {
				logger::error("Failed to register window class. Error: ", error, '.') ;
				return false ;
			}
		}

		logger::info("Window class registered successfully with atom: ", result) ;
		__App_Registry__::g_was_registered_ = true ;
		return true ;
	}
}