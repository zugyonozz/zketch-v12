#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>

#include "logger.h"
#include "event_.h"

namespace zketch {

	using EventHandler = std::function<void(const Event&)> ;

	class Application ;
	class Window ;

	static inline bool IS_RUNNING = true ;
	static constexpr const char* WNDCLASSNAME = "ZKETCH_APPLICATION" ;
	static inline std::unordered_map<HWND, std::unique_ptr<Window>> g_windows_ ;
	static inline std::unordered_map<HWND, std::queue<Event>> g_window_events_ ;
	static inline std::unordered_map<HWND, EventHandler> g_window_handlers_ ;
	static inline std::queue<Event> g_events_ ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;

	public :
		Window(const char* title, int width, int height, HINSTANCE h_instance_) noexcept {
			hwnd_ = CreateWindowExA(
				0,         
				WNDCLASSNAME,
				title,               
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				width,
				height,
				nullptr,
				nullptr,
				h_instance_,
				this
			) ;
		}

		~Window() noexcept {
			if (hwnd_)
				DestroyWindow(hwnd_) ;
		}

		const HWND get_handle() const noexcept {
			return hwnd_ ;
		}

		LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) noexcept {
			switch (msg) {
				case WM_DESTROY :
					return 0;
				default :
					return DefWindowProc(hwnd_, msg, wp, lp);
			}
		}
	} ;

	static inline void DrainMessages() noexcept {
		MSG msg{} ;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				g_events_.push(Event::createEvent(nullptr, EventType::Quit));
			} else {
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}
		}
	}

	class Application {
	private :
		HINSTANCE h_instance_ ;

		bool RegisterWindowClass() const noexcept{
			WNDCLASSEX wc{} ;
			wc.cbSize = sizeof(WNDCLASSEX) ;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
			wc.lpfnWndProc = WindowProcedureSetup ;
			wc.hInstance = h_instance_ ;
			wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION) ;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW) ;
			wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1) ;
			wc.lpszClassName = WNDCLASSNAME ;
			wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION) ;

			if (RegisterClassEx(&wc) == 0) {
				logger::error("Failed to Register window class\nError\t:", GetLastError(), ".\n") ;
				return false ;
			}
			logger::info("Window class registered successfully.\n") ;
			return true ;
		}

		static LRESULT CALLBACK WindowProcedureSetup(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
            if (msg == WM_NCCREATE) {
                const CREATESTRUCTA* const p_create = reinterpret_cast<CREATESTRUCTA*>(lp) ;
                if (!p_create) 
					return FALSE ;
                
                Window* window_ = static_cast<Window*>(p_create->lpCreateParams) ;
                if (!window_) 
					return FALSE ;
                
                SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_)) ;
                SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::WindowProcedure)) ;
                
                return window_->HandleMessage(msg, wp, lp) ;
            }
            
            return DefWindowProcA(hwnd, msg, wp, lp) ;
        }

	public :
		Application() noexcept : h_instance_(GetModuleHandleA(nullptr)) {
			RegisterWindowClass() ;
		}

		~Application() {
			UnregisterClass(WNDCLASSNAME, h_instance_) ;
		}

		void AddHandler(HWND hwnd_, EventHandler handler) noexcept {
			g_window_handlers_[hwnd_] = handler ;
		}

		void CleareHandler(HWND hwnd_) noexcept {
			g_window_handlers_.erase(hwnd_) ;
		}

		HWND createNewWindow(const char* title, int width, int height) noexcept {
			auto window_ = std::make_unique<Window>(title, width, height, h_instance_) ;
			HWND hwnd_ = window_->get_handle() ;
			if (hwnd_) {
				ShowWindow(hwnd_, SW_SHOW) ;
				UpdateWindow(hwnd_) ;
				g_windows_[hwnd_] = std::move(window_) ;
			}
			return hwnd_ ;
		}

		static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
			Event event_ = Event::getEvent({hwnd, msg, wp, lp, 0, {0, 0}}) ;
			if (event_ != EventType::None)
				g_window_events_[hwnd].push(event_) ;
			auto* window_ = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) ;
			if (window_) 
				return window_->HandleMessage(msg, wp, lp) ;
			return DefWindowProc(hwnd, msg, wp, lp) ;
		}

		static bool pollEvent(HWND hwnd, Event& out) noexcept {
			DrainMessages() ;

			auto it = g_window_events_.find(hwnd) ;
			if (it != g_window_events_.end() && !it->second.empty()) {
				out = it->second.front() ;
				it->second.pop() ;
				return true ;
			}
			return false ;
		}

		static bool pollGlobalEvent(Event& out) noexcept {
			DrainMessages() ;

			if (!g_events_.empty()) { 
				out = g_events_.front() ; 
				g_events_.pop() ; 
				return true; 
			}
			return false;
		}

		int Run() noexcept {
			while(IS_RUNNING) {
				for(auto& h : g_window_events_) {
					Event event_ = Event::createEvent() ;
					if (pollEvent(h.first, event_)) {
						g_window_handlers_[h.first](event_) ;
					}
				}
			}
		}
	} ;
}