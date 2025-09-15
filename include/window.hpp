#pragma once

#include "renderer.hpp"
#include "inputsystem.hpp"

namespace zketch {

	class Window ;
	class TrackBar ;

	class Application_ {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Window ;
		friend class TrackBar ;
	private :
		static inline std::unordered_map<HWND, Window*> g_windows_ ;
		static inline std::unordered_map<HWND, TrackBar*> g_trackbars_ ;
		
		bool is_run_ = true ;
	
	public :

		constexpr Application_() noexcept = default ;
		void QuitProgram() noexcept {
			for (auto& w : g_windows_) {
				if (IsWindow(w.first)) {
					DestroyWindow(w.first) ;
				}
			}
			g_windows_.clear() ;
			is_run_ = false ;
			PostQuitMessage(0) ;
			logger::info("PostQuitMessage (QuitProgram) done") ;
		}

		void QuitWindow(HWND hwnd) noexcept {
			if (hwnd && IsWindow(hwnd)) {
				SendMessage(hwnd, WM_CLOSE, 0, 0) ;
				logger::info("SendMessage WM_CLOSE for hwnd") ;
			}
		}

		constexpr operator bool() const noexcept {
			return is_run_ ;
		}
	} static Application ;

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
			case WM_SIZE : 
				logger::info("WM SIZE received") ;
				return 0 ;
			case WM_CLOSE : 
				logger::info("WM CLOSE received") ;
				DestroyWindow(hwnd) ;
				return 0 ;
			case WM_DESTROY : 
				if (hwnd) {
					auto it = Application_::g_windows_.find(hwnd) ;
					if (it != Application_::g_windows_.end()) {
						Application_::g_windows_.erase(it) ;
						logger::info("erased window from g_windows_. current size : ", Application_::g_windows_.size()) ;
					} else {
						logger::info("WM_DESTROY for hwnd not found in g_windows_. size: ", Application_::g_windows_.size()) ;
					}
					logger::info("current g_windows size : ", Application_::g_windows_.size()) ;
				}
				if (Application_::g_windows_.empty()) {
					Application.QuitProgram() ;
				}
				return 0 ;
		}
		return DefWindowProc(hwnd, msg, wp, lp) ;
	}

	namespace AppRegistry {

		static inline HINSTANCE g_hintance_ = GetModuleHandleW(nullptr) ;
		static inline std::string g_window_class_name_ = "zketch_app" ;
		static inline bool window_was_registered = false ;
		static inline bool menu_was_registered = false ;

		static void SetWindowClass(std::string&& windowclassname) noexcept {
			if (window_was_registered) {
				logger::warning("SetWindowClass() failed : window class name was registered.") ;
			} else {
				g_window_class_name_ = std::move(windowclassname) ;
			}
		}

		static void RegisterWindowClass() {
			if (window_was_registered) {
				logger::warning("RegisterWindowClass() failed : window class name was registered.") ;
			} else {
				WNDCLASSEX wc = {
					sizeof(wc),
					CS_HREDRAW | CS_VREDRAW,
					wndproc,
					0,
					0,
					AppRegistry::g_hintance_,
					LoadIcon(nullptr, IDI_APPLICATION),
					LoadCursor(nullptr, IDC_ARROW),
					reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
					nullptr,
					AppRegistry::g_window_class_name_.c_str(),
					LoadIcon(nullptr, IDI_APPLICATION)
				} ;

				if (!RegisterClassEx(&wc)) {
					logger::error("RegisterWindowClass() failed : Error to registering window class!") ;
				} else {
					logger::info("RegisterWindowClass() success") ;
					window_was_registered = true ;
				}
			}
		}

		static void RegisterCommonControl() {
			if (menu_was_registered) {
				logger::warning("RegisterCommonControl() failed : common control was registered.") ;
			} else {
				tagINITCOMMONCONTROLSEX icex{
					sizeof(icex),
					0xFFFF // langsung regist semua
				} ;

				InitCommonControlsEx(&icex) ;
			}
		}

		// next improvement is adding RegisterWindowClass with custom Style, icon, cursor, and other...
		// next improvement is adding RegisterCommonControls with custom dxICC
	} ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, int width, int height) noexcept {
			hwnd_ = CreateWindowEx(
				0,
				AppRegistry::g_window_class_name_.c_str(),
				title,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				width,
				height,
				nullptr,
				nullptr,
				AppRegistry::g_hintance_,
				nullptr
			) ;

			if (!hwnd_) 
				logger::error("Window() failed to create Window : window isn't valid") ;
			Application_::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window() create Window success") ;
		}

		Window(const char* title, int xpos, int ypos, int width, int height) noexcept {
			hwnd_ = CreateWindowEx(
				0,
				AppRegistry::g_window_class_name_.c_str(),
				title,
				WS_OVERLAPPEDWINDOW,
				xpos,
				ypos,
				width,
				height,
				nullptr,
				nullptr,
				AppRegistry::g_hintance_,
				nullptr
			) ;

			if (!hwnd_) 
				logger::error("Window() failed to create Window : window isn't valid") ;
			Application_::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window() create Window success") ;
		}

		Window(Window&& o) noexcept : hwnd_(std::exchange(o.hwnd_, nullptr)) {
			logger::info("Calling move ctor.") ;
			if (hwnd_) {
				auto it = Application_::g_windows_.find(o.hwnd_) ;
				if (it != Application_::g_windows_.end()) 
					Application_::g_windows_.erase(it) ;
				Application_::g_windows_[hwnd_] = this ;
			}
		}

		~Window() noexcept {
			logger::info("Calling dtor of class window") ;
			if (hwnd_) {
				if (IsWindow(hwnd_)) {
					DestroyWindow(hwnd_) ;
				}
				hwnd_ = nullptr ;
				logger::info("Success to set hwnd to nullptr.") ;
			} else {
				logger::warning("failed to set hwnd, hwnd is already nullptr.") ;
			}
		}

		Window& operator=(Window&& o) noexcept {
			logger::info("Calling move assignment.") ;
			if (this != &o) {
				hwnd_ = std::exchange(o.hwnd_, nullptr) ;
				auto it = Application_::g_windows_.find(o.hwnd_) ;
				if (it != Application_::g_windows_.end()) 
					Application_::g_windows_.erase(it) ;
				Application_::g_windows_[hwnd_] = this ;
			}
			return *this ;
		}

		HWND getHandle() const noexcept {
			return hwnd_ ;
		}

		bool IsValid() const noexcept {
			return hwnd_ != nullptr ;
		}

		void Show() const noexcept {
			if (hwnd_) {
				ShowWindow(hwnd_, SW_SHOWDEFAULT) ;
				UpdateWindow(hwnd_) ;
			}
		}

		void Hide() const noexcept {
			if (hwnd_) 
				ShowWindow(hwnd_, SW_HIDE) ; 
		}

		void minimize() noexcept { 
			if (hwnd_) 
				ShowWindow(hwnd_, SW_MINIMIZE) ; 
		}

		void maximize() noexcept { 
			if (hwnd_) 
				ShowWindow(hwnd_, SW_MAXIMIZE) ; 
		}

		void restore() noexcept { 
			if (hwnd_) 
				ShowWindow(hwnd_, SW_RESTORE) ; 
		}

		void Quit() const noexcept {
			if (hwnd_) {
				logger::info("Quit() called for window: ", hwnd_) ;
				DestroyWindow(hwnd_);
			} else {
				logger::warning("Quit() failed: invalid window handle") ;
			}
		}

		Rect getClientBound() const noexcept {
			tagRECT r ;
			GetClientRect(hwnd_, &r) ;
			return static_cast<Rect>(r) ;
		}

		Rect getClipBound() const noexcept {
			tagRECT r ;
			GetWindowRect(hwnd_, &r) ;
			return static_cast<Rect>(r) ;
		}

		void setTitle(const char* title) noexcept {
			if (hwnd_)
				SetWindowText(hwnd_, title) ;
		}
	} ;

	class TrackBar {
	private :
		HWND hwnd_ = nullptr ;

	public :
		TrackBar(const TrackBar&) = delete ;
		TrackBar& operator=(const TrackBar&) = delete ;

		TrackBar(HWND parent, const Rect& rect) {
			hwnd_ = CreateWindowEx(
				0,
				TRACKBAR_CLASS,
				"",
				WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
				rect.x,
				rect.y,
				rect.w,
				rect.h,
				parent,
				reinterpret_cast<HMENU>(static_cast<size_t>(ID_MENU)),
				AppRegistry::g_hintance_,
				nullptr
			) ;
			if (!hwnd_) 
				logger::error("TrackBar() failed to create trackbar : handle is nullptr") ;
			Application_::g_trackbars_.emplace(hwnd_, this) ;
			logger::info("TrackBar() create trackbar success") ;

			setPos(0) ;
		}

		~TrackBar() noexcept {
			logger::info("Calling dtor of class TrackBar") ;
			if (hwnd_) {
				if (IsWindow(hwnd_)) {
					DestroyWindow(hwnd_) ;
				}
				hwnd_ = nullptr ;
				logger::info("Success to set hwnd to nullptr.") ;
			} else {
				logger::warning("failed to set hwnd, hwnd is already nullptr.") ;
			}
		}

		int getPos() const noexcept {
			return static_cast<int>(SendMessage(hwnd_, TBM_GETPOS, 0, 0));
		}

		HWND getHandle() const noexcept {
			return hwnd_ ;
		}

		void setRange(int min, int max, bool redraw = true) noexcept {
			SendMessage(hwnd_, TBM_SETRANGE, redraw, MAKELPARAM(min, max));
		}

		void setPos(int pos, bool redraw = true) noexcept {
			SendMessage(hwnd_, TBM_SETPOS, redraw, pos);
		}

		void setTickFreq(int freq) noexcept {
			SendMessage(hwnd_, TBM_SETTICFREQ, freq, 0);
		}
	} ;
}