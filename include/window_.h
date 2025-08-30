#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>

#include "event_.h"
#include "texture.h"

namespace zketch {

	using EventHandler = std::function<void(const Event&)> ;

	class Application ;
	class Window ;

	static inline bool IS_RUNNING = true ;
	static constexpr const char* WNDCLASSNAME = "ZKETCH_APPLICATION" ;
	static inline std::unordered_map<HWND, std::unique_ptr<Window>> g_windows_ ;
	static inline std::unordered_map<HWND, std::queue<Event>> g_window_events_ ;
	static inline std::unordered_map<HWND, EventHandler> g_window_handlers_ ;
	static inline std::unordered_map<HWND, std::vector<Texture>> g_window_textures_ ; 
	static inline std::queue<Event> g_events_ ;

	// Forward declare the window procedure
	LRESULT CALLBACK GlobalWindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;

	public :
		Window(const char* title, int width, int height, HINSTANCE h_instance_) noexcept {
			logger::info("Attempting to create window: ", title) ;
			logger::info("Instance handle: ", h_instance_) ;
			
			hwnd_ = CreateWindowEx(
				0,                      // Extended window style
				WNDCLASSNAME,          // Window class name
				title,                 // Window title
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,  // Window style with visible flag
				CW_USEDEFAULT,         // X position
				CW_USEDEFAULT,         // Y position
				width,                 // Width
				height,                // Height
				nullptr,               // Parent window
				nullptr,               // Menu
				h_instance_,           // Instance handle
				this                   // Additional application data
			) ;
			
			if (!hwnd_) {
				DWORD error = GetLastError() ;
				logger::error("Failed to create window. Error code: ", error) ;
				
				// Try to get more detailed error information
				LPSTR messageBuffer = nullptr ;
				size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
											 NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL) ;
				
				if (size > 0) {
					logger::error("Error description: ", messageBuffer) ;
					LocalFree(messageBuffer) ;
				}
			} else {
				logger::info("Window created successfully with handle: ", hwnd_) ;
				
				// Initialize event queue for this window
				g_window_events_[hwnd_] = std::queue<Event>() ;
				
				// Show and update the window
				ShowWindow(hwnd_, SW_SHOWDEFAULT) ; 
				UpdateWindow(hwnd_) ;
				SetForegroundWindow(hwnd_) ;
				SetFocus(hwnd_) ;
			}
		}

		~Window() noexcept {
			if (hwnd_) {
				logger::info("Destroying window: ", hwnd_) ;
				DestroyWindow(hwnd_) ;
				hwnd_ = nullptr ;
			}
		}

		HWND get_handle() const noexcept {
			return hwnd_ ;
		}

		LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp) noexcept {
			switch (msg) {
				case WM_DESTROY :
					logger::info("WM_DESTROY received for window: ", hwnd_) ;
					PostQuitMessage(0) ;
					return 0;
					
				case WM_CLOSE :
					logger::info("WM_CLOSE received for window: ", hwnd_) ;
					DestroyWindow(hwnd_) ;
					return 0;
					
				case WM_PAINT : {
					PAINTSTRUCT ps ;
					HDC hdc = BeginPaint(hwnd_, &ps) ;
					// Fill with white background
					FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1)) ;
					EndPaint(hwnd_, &ps) ;
					return 0;
				}
				
				default :
					return DefWindowProcA(hwnd_, msg, wp, lp);
			}
		}
	} ;

	static inline void DrainMessages() noexcept {
		MSG msg{} ;
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				logger::info("WM_QUIT message received") ;
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
		bool class_registered_ = false ;

		bool RegisterWindowClass() noexcept {
			logger::info("Registering window class...") ;
			
			WNDCLASSEX wc{} ;
			wc.cbSize = sizeof(WNDCLASSEX) ;
			wc.style = CS_HREDRAW | CS_VREDRAW ;
			wc.lpfnWndProc = GlobalWindowProcedure ;
			wc.cbClsExtra = 0 ;
			wc.cbWndExtra = 0 ;
			wc.hInstance = h_instance_ ;
			wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION) ;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW) ;
			wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1) ;
			wc.lpszMenuName = nullptr ;
			wc.lpszClassName = WNDCLASSNAME ;
			wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION) ;

			ATOM result = RegisterClassEx(&wc) ;
			if (result == 0) {
				DWORD error = GetLastError() ;
				if (error == ERROR_CLASS_ALREADY_EXISTS) {
					logger::info("Window class already exists, continuing...") ;
					class_registered_ = true ;
					return true ;
				} else {
					logger::error("Failed to register window class. Error: ", error) ;
					return false ;
				}
			}
			
			logger::info("Window class registered successfully with atom: ", result) ;
			class_registered_ = true ;
			return true ;
		}

	public :
		Application() noexcept : h_instance_(GetModuleHandle(nullptr)) {
			logger::info("Initializing application...") ;
			logger::info("Module handle: ", h_instance_) ;
			
			if (!h_instance_) {
				logger::error("Failed to get module handle") ;
				return ;
			}
			
			if (!RegisterWindowClass()) {
				logger::error("Application initialization failed!") ;
			} else {
				logger::info("Application initialized successfully") ;
			}
		}

		~Application() noexcept {
			logger::info("Destroying application...") ;
			
			// Clear all windows first
			g_windows_.clear() ;
			g_window_events_.clear() ;
			g_window_handlers_.clear() ;
			
			// Unregister class if we registered it
			if (class_registered_) {
				if (UnregisterClassA(WNDCLASSNAME, h_instance_)) {
					logger::info("Window class unregistered successfully") ;
				} else {
					logger::error("Failed to unregister window class. Error: ", GetLastError()) ;
				}
			}
			
			logger::info("Application destroyed") ;
		}

		void AddHandler(HWND hwnd_, EventHandler handler) noexcept {
			if (hwnd_ && handler) {
				g_window_handlers_[hwnd_] = handler ;
				logger::info("Event handler added for window: ", hwnd_) ;
			} else {
				logger::error("Invalid parameters for AddHandler") ;
			}
		}

		void ClearHandler(HWND hwnd_) noexcept {
			auto it = g_window_handlers_.find(hwnd_) ;
			if (it != g_window_handlers_.end()) {
				g_window_handlers_.erase(it) ;
				logger::info("Event handler cleared for window: ", hwnd_) ;
			}
		}

		void AddTexture(HWND hwnd_, Texture* texture_) noexcept {
			if (hwnd_ && !texture_->Empty()) {
				g_window_textures_[hwnd_].push_back(*texture_) ;
				logger::info("Texture added for window: ", hwnd_) ;
			} else {
				logger::error("Invalid parameters for AddTexture") ;
			}
		}

		void DeleteTexture(HWND hwnd_, size_t index_) noexcept {
			auto it = g_window_textures_.find(hwnd_) ;
			if (it != g_window_textures_.end()) {
				it->second.erase(it->second.begin() + index_) ;
				logger::info("Erased texture for window: ", hwnd_, " at index:", index_) ;
			}
		}

		HWND createNewWindow(const char* title, int width, int height) noexcept {
			if (!class_registered_) {
				logger::error("Cannot create window: class not registered") ;
				return nullptr ;
			}
			
			if (!title) {
				logger::error("Window title cannot be null") ;
				return nullptr ;
			}
			
			logger::info("Creating window: ", title, " (", width, "x", height, ")") ;
			
			auto window_ = std::make_unique<Window>(title, width, height, h_instance_) ;
			HWND hwnd_ = window_->get_handle() ;
			
			if (hwnd_) {
				g_windows_[hwnd_] = std::move(window_) ;
				logger::info("Window added to collection: ", hwnd_) ;
				return hwnd_ ;
			} else {
				logger::error("Window creation failed") ;
				return nullptr ;
			}
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
			logger::info("Starting main application loop...") ;
			
			if (g_windows_.empty()) {
				logger::error("No windows to process, exiting") ;
				return -1 ;
			}
			
			while (IS_RUNNING && !g_windows_.empty()) {
				// Process global events first
				Event globalEvent = Event::createEvent() ;
				while (pollGlobalEvent(globalEvent)) {
					if (globalEvent == EventType::Quit) {
						logger::info("Global quit event received") ;
						IS_RUNNING = false ;
						break ;
					}
				}
				
				if (!IS_RUNNING) break ;
				
				// Process window-specific events
				bool hasValidWindows = false ;
				for (auto it = g_window_events_.begin(); it != g_window_events_.end(); ) {
					HWND hwnd = it->first ;
					
					// Check if window still exists
					if (!IsWindow(hwnd)) {
						logger::info("Window no longer exists, cleaning up: ", hwnd) ;
						g_window_handlers_.erase(hwnd) ;
						g_windows_.erase(hwnd) ;
						it = g_window_events_.erase(it) ;
						continue ;
					}
					
					hasValidWindows = true ;
					
					Event event_ = Event::createEvent() ;
					while (pollEvent(hwnd, event_)) {
						auto handlerIt = g_window_handlers_.find(hwnd) ;
						if (handlerIt != g_window_handlers_.end()) {
							handlerIt->second(event_) ;
						}
						
						// Handle quit events
						if (event_ == EventType::Quit || event_ == EventType::Close) {
							logger::info("Quit/Close event received for window: ", hwnd) ;
							IS_RUNNING = false ;
							break ;
						}
					}

					if (!g_window_textures_.empty()) {
						if (!g_window_textures_[it->first].empty())
							for (auto& t : g_window_textures_.at(it->first))
								Renderer::Present(it->first, t, Point{}) ;
					}
					
					if (!IS_RUNNING) break ;
					++it ;
				}
				
				if (!hasValidWindows) {
					logger::info("No valid windows remaining") ;
					break ;
				}
				
				// Small sleep to prevent 100% CPU usage
				Sleep(160) ;
			}
			
			logger::info("Application main loop ended") ;
			return 0 ;
		}
	} ;

	// Global window procedure implementation
	LRESULT CALLBACK GlobalWindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
		// Handle window creation
		if (msg == WM_NCCREATE) {
			const CREATESTRUCTA* const p_create = reinterpret_cast<CREATESTRUCTA*>(lp) ;
			if (p_create && p_create->lpCreateParams) {
				Window* window_ = static_cast<Window*>(p_create->lpCreateParams) ;
				SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_)) ;
			}
			return DefWindowProcA(hwnd, msg, wp, lp) ;
		}
		
		// Get the window instance
		Window* window_ = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)) ;
		
		// Create and queue the event
		Event event_ = Event::getEvent({hwnd, msg, wp, lp, 0, {0, 0}}) ;
		if (event_ != EventType::None) {
			auto it = g_window_events_.find(hwnd) ;
			if (it != g_window_events_.end()) {
				it->second.push(event_) ;
			}
		}
		
		// Let the window handle the message
		if (window_) {
			return window_->HandleMessage(msg, wp, lp) ;
		}
		
		return DefWindowProcA(hwnd, msg, wp, lp) ;
	}
}