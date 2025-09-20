#pragma once

#include "renderer.hpp"
#include "inputsystem.hpp"

namespace zketch {

	class Window ;
	class TrackBar ;

	class Application_ {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Window ;
	private :
		static inline std::unordered_map<HWND, Window*> g_windows_ ;
		
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
			case WM_HSCROLL : 
				logger::info("-> Converting to HScroll event") ;
				if (lp != 0) { 
					size_t pos = static_cast<size_t>(SendMessage(reinterpret_cast<HWND>(lp), TBM_GETPOS, 0, 0)) ;
					EventSystem::PushEvent(Event::createScrollEvent(reinterpret_cast<HWND>(lp), TrackBarType::HScroll, pos)) ;
				}
				break;
			case WM_VSCROLL : 
				logger::info("-> Converting to VScroll event") ;
				if (lp != 0) { 
					size_t pos = static_cast<size_t>(SendMessage(reinterpret_cast<HWND>(lp), TBM_GETPOS, 0, 0)) ;
					EventSystem::PushEvent(Event::createScrollEvent(reinterpret_cast<HWND>(lp), TrackBarType::VScroll, pos)) ;
				}
				break ;
			case WM_SIZE : 
				logger::info("-> Converting to Resize event (width=", LOWORD(lp), ", height=", HIWORD(lp), ")") ;
				EventSystem::PushEvent(Event::createResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
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

		// next improvement is adding RegisterWindowClass with custom Style, icon, cursor, and other...
	} ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, int32_t width, int32_t height) noexcept {
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

		Window(const char* title, int32_t xpos, int32_t ypos, int32_t width, int32_t height) noexcept {
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

		HWND GetHandle() const noexcept {
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

		void Minimize() noexcept { 
			if (hwnd_) 
				ShowWindow(hwnd_, SW_MINIMIZE) ; 
		}

		void Maximize() noexcept { 
			if (hwnd_) 
				ShowWindow(hwnd_, SW_MAXIMIZE) ; 
		}

		void Restore() noexcept { 
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

		Rect GetClientBound() const noexcept {
			tagRECT r ;
			GetClientRect(hwnd_, &r) ;
			return static_cast<Rect>(r) ;
		}

		Rect GetClipBound() const noexcept {
			tagRECT r ;
			GetWindowRect(hwnd_, &r) ;
			return static_cast<Rect>(r) ;
		}

		void SetTitle(const char* title) noexcept {
			if (hwnd_)
				SetWindowText(hwnd_, title) ;
		}
	} ;

	class Slider {
	public :
		enum Orientation : uint8_t {
			Vertical,
			Horizontal
		} ;

		struct Style {
			Color background_ = rgba(0, 0, 0, 0) ;
			Color border_fill_ = rgba(0, 0, 0, 0) ;
			Color border_stroke_ = rgba(0, 0, 0, 0) ;
			Color thumb_fill_ = rgba(0, 0, 0, 0) ;
			Color thumb_stroke_ = rgba(0, 0, 0, 0) ;
			Rect border_bound_ = {} ;
			Rect thumb_bound_ = {} ;
			Shape border_shape = Shape::Rect ;
			Shape thumb_shape = Shape::Rect ;
			float border_thick_ = 0.0f ;
			float thumb_thick_ = 0.0f ;
		} ;

	private :
		Orientation orientation_ ;
		Canvas* canvas_ ;
		Rect* canvas_bound_ ;
		Rect* thumb_bound_ ;
		bool on_drag_ ;
		Style* style_ ;

		void Update() noexcept {
			Drawer draw ;
			if (!draw.Begin(*canvas_))
				return ;
			draw.Clear(style_->background_) ;
			draw.FillRect(style_->border_bound_, rgba(255, 0, 0, 1)) ;
			if (style_->border_thick_ > 0.0f) {
				draw.DrawRect(style_->border_bound_, style_->border_stroke_) ;
			}
			draw.FillRect(style_->thumb_bound_, style_->thumb_fill_) ;
			if (style_->thumb_thick_ > 0.0f) {
				draw.DrawRect(style_->thumb_bound_, style_->thumb_stroke_) ;
			}
			draw.End() ;
		}

	public :
		Slider(Orientation orientation, const Rect& bound, const Size& thumb) noexcept {
			orientation_ = orientation ;
			canvas_ = new Canvas() ;
			canvas_->Create(bound.getSize()) ;
			canvas_bound_ = new Rect() ;
			*canvas_bound_ = bound ;
			thumb_bound_ = new Rect() ;
			*thumb_bound_ = {Point_<uint32_t>{0, 0}, thumb} ;
			on_drag_ = false ;
			style_ = new Style() ;
			Update() ;
		}

		~Slider() noexcept {
			delete canvas_ ;
			delete thumb_bound_ ;
			delete canvas_bound_ ;
			delete style_ ;
		}

		void OnMouseDown(const Point& pos) noexcept {
			if (thumb_bound_->Contain(pos)) {
				on_drag_ = true ;
				Update() ;
			}
		}

		void OnMouseMove(const Point& pos) noexcept {
			if (!on_drag_) {
				return ;
			}

			if (orientation_ == Horizontal) {
				thumb_bound_->x = pos.x ;
			} else {
				thumb_bound_->y = pos.y ;
			}
			Update() ;
		}

		void OnMouseUp() noexcept {
			on_drag_ = false ;
			Update() ;
		}

		template <typename T = int32_t, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		T GetValue() const noexcept {
			if constexpr (std::is_integral_v<T>) {
				return orientation_ == Horizontal ? thumb_bound_->x : thumb_bound_->y ;
			}
			return orientation_ == Horizontal ? (static_cast<T>(thumb_bound_->x) / (canvas_bound_->w - thumb_bound_->w) * 100.0f) : (static_cast<T>(thumb_bound_->y) / (canvas_bound_->h - thumb_bound_->y) * 100.0f) ;
		}

		void Present(HWND hwnd) const noexcept {
			canvas_->Present(hwnd) ;
		}

		bool OnDrag() const noexcept {
			return on_drag_ ;
		}
	} ;
}