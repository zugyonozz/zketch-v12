#pragma once

#include "renderer copy.hpp"
#include "event.hpp"

namespace zketch {

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

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
					CS_HREDRAW | CS_VREDRAW | CS_OWNDC,  // Added CS_OWNDC
					wndproc,
					0,
					0,
					AppRegistry::g_hintance_,
					LoadIcon(nullptr, IDI_APPLICATION),
					LoadCursor(nullptr, IDC_ARROW),
					nullptr,  // Changed from COLOR_WINDOW+1 to nullptr
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
	} ;

	class Window {
	private :
		HWND hwnd_ = nullptr ;
		std::unique_ptr<Canvas> back_buffer_ ;  // Renamed for clarity
		HDC hdc_ = nullptr ;
		HDC mem_dc_ = nullptr ;
		HBITMAP mem_bitmap_ = nullptr ;
		HBITMAP old_bitmap_ = nullptr ;
		int32_t width_ = 0 ;
		int32_t height_ = 0 ;
		bool needs_redraw_ = true ;

		void InitializeBuffers(int32_t width, int32_t height) noexcept {
			width_ = width ;
			height_ = height ;

			// Create back buffer canvas
			back_buffer_ = std::make_unique<Canvas>() ;
			if (!back_buffer_->Create({width, height})) {
				logger::error("Window::InitializeBuffers - failed to create back buffer canvas") ;
				return ;
			}

			// Set transparent clear color
			back_buffer_->SetClearColor(rgba(255, 255, 255, 255)) ;

			// Get window DC
			hdc_ = GetDC(hwnd_) ;
			if (!hdc_) {
				logger::error("Window::InitializeBuffers - GetDC failed") ;
				return ;
			}

			// Create memory DC for double buffering
			mem_dc_ = CreateCompatibleDC(hdc_) ;
			if (!mem_dc_) {
				logger::error("Window::InitializeBuffers - CreateCompatibleDC failed") ;
				return ;
			}

			// Create compatible bitmap
			mem_bitmap_ = CreateCompatibleBitmap(hdc_, width, height) ;
			if (!mem_bitmap_) {
				logger::error("Window::InitializeBuffers - CreateCompatibleBitmap failed") ;
				return ;
			}

			// Select bitmap into memory DC
			old_bitmap_ = (HBITMAP)SelectObject(mem_dc_, mem_bitmap_) ;

			logger::info("Window buffers initialized: ", width, "x", height) ;
		}

		void CleanupBuffers() noexcept {
			if (mem_dc_) {
				if (old_bitmap_) {
					SelectObject(mem_dc_, old_bitmap_) ;
					old_bitmap_ = nullptr ;
				}
				DeleteDC(mem_dc_) ;
				mem_dc_ = nullptr ;
			}

			if (mem_bitmap_) {
				DeleteObject(mem_bitmap_) ;
				mem_bitmap_ = nullptr ;
			}

			if (hdc_ && hwnd_) {
				ReleaseDC(hwnd_, hdc_) ;
				hdc_ = nullptr ;
			}
		}

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

			if (!hwnd_) {
				logger::error("Window() failed to create Window : window isn't valid") ;
				return ;
			}

			Application_::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window() create Window success") ;

			// Get actual client area size
			RECT rc ;
			GetClientRect(hwnd_, &rc) ;
			InitializeBuffers(rc.right - rc.left, rc.bottom - rc.top) ;
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

			if (!hwnd_) {
				logger::error("Window() failed to create Window : window isn't valid") ;
				return ;
			}

			Application_::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window() create Window success") ;

			// Get actual client area size
			RECT rc ;
			GetClientRect(hwnd_, &rc) ;
			InitializeBuffers(rc.right - rc.left, rc.bottom - rc.top) ;
		}

		Window(Window&& o) noexcept 
			: hwnd_(std::exchange(o.hwnd_, nullptr))
			, back_buffer_(std::move(o.back_buffer_))
			, hdc_(std::exchange(o.hdc_, nullptr))
			, mem_dc_(std::exchange(o.mem_dc_, nullptr))
			, mem_bitmap_(std::exchange(o.mem_bitmap_, nullptr))
			, old_bitmap_(std::exchange(o.old_bitmap_, nullptr))
			, width_(o.width_)
			, height_(o.height_) 
		{
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
			CleanupBuffers() ;
			
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
				CleanupBuffers() ;
				
				hwnd_ = std::exchange(o.hwnd_, nullptr) ;
				back_buffer_ = std::move(o.back_buffer_) ;
				hdc_ = std::exchange(o.hdc_, nullptr) ;
				mem_dc_ = std::exchange(o.mem_dc_, nullptr) ;
				mem_bitmap_ = std::exchange(o.mem_bitmap_, nullptr) ;
				old_bitmap_ = std::exchange(o.old_bitmap_, nullptr) ;
				width_ = o.width_ ;
				height_ = o.height_ ;
				
				auto it = Application_::g_windows_.find(o.hwnd_) ;
				if (it != Application_::g_windows_.end()) 
					Application_::g_windows_.erase(it) ;
				Application_::g_windows_[hwnd_] = this ;
			}
			return *this ;
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

		operator HWND() const noexcept {
			return hwnd_ ;
		}

		void SetTitle(const char* title) noexcept {
			if (hwnd_)
				SetWindowText(hwnd_, title) ;
		}

		// Clear the back buffer before drawing
		void BeginDraw() noexcept {
			if (!back_buffer_ || !back_buffer_->IsValid()) {
				logger::warning("Window::BeginDraw - invalid back buffer") ;
				return ;
			}

			Drawer drawer ;
			if (drawer.Begin(*back_buffer_, true)) {
				drawer.End() ;
			}
			needs_redraw_ = true ;
		}

		// Present a canvas to the back buffer at position
		void Present(const Canvas& canvas, const Point& pos) noexcept {
			if (!back_buffer_ || !back_buffer_->IsValid()) {
				logger::warning("Window::Present - invalid back buffer") ;
				return ;
			}

			auto* bitmap = canvas.GetBitmap() ;
			if (!bitmap) {
				logger::warning("Window::Present - source canvas has no bitmap") ;
				return ;
			}

			// Draw to back buffer using Drawer
			Drawer drawer ;
			if (drawer.Begin(*back_buffer_, false)) {  // Don't auto-clear
				drawer.DrawCanvas(&canvas, static_cast<float>(pos.x), static_cast<float>(pos.y)) ;
				drawer.End() ;
			}
			
			needs_redraw_ = true ;
		}

		// Present canvas at (0,0)
		void Present(const Canvas& canvas) noexcept {
			Present(canvas, {0, 0}) ;
		}

		// Present multiple canvases
		template <typename ... Canvases, typename = std::enable_if_t<(std::is_same_v<Canvases, Canvas> && ...)>>
		void Present(const Canvas& canvas, const Canvases& ... canvases) noexcept {
			Present(canvas) ;
			Present(canvases...) ;
		}

		// Actually display the back buffer to screen
		void Display() noexcept {
			if (!back_buffer_ || !back_buffer_->IsValid()) {
				logger::warning("Window::Display - invalid back buffer") ;
				return ;
			}

			if (!needs_redraw_) {
				return ;
			}

			// Copy back buffer to memory DC using GDI+
			Gdiplus::Graphics gfx(mem_dc_) ;
			gfx.SetCompositingMode(Gdiplus::CompositingModeSourceCopy) ;
			gfx.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			gfx.DrawImage(back_buffer_->GetBitmap(), 0, 0, width_, height_) ;

			// Blit to window
			BitBlt(hdc_, 0, 0, width_, height_, mem_dc_, 0, 0, SRCCOPY) ;
			
			needs_redraw_ = false ;
		}

		// Called from WM_PAINT
		void Redraw() noexcept {
			Display() ;
		}

		// Force a redraw
		void Invalidate() noexcept {
			needs_redraw_ = true ;
			if (hwnd_) {
				InvalidateRect(hwnd_, nullptr, FALSE) ;
			}
		}
	} ;

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
			case WM_SIZE : 
				EventSystem::PushEvent(Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
			case WM_PAINT : {
				// Handle paint messages properly
				PAINTSTRUCT ps ;
				BeginPaint(hwnd, &ps) ;
				
				auto it = Application_::g_windows_.find(hwnd) ;
				if (it != Application_::g_windows_.end() && it->second) {
					it->second->Redraw() ;
				}
				
				EndPaint(hwnd, &ps) ;
				return 0 ;
			}
			case WM_ERASEBKGND :
				// Prevent erasing background to reduce flicker
				return 1 ;
			case WM_CLOSE : 
				EventSystem::PushEvent(Event::CreateNormalEvent(hwnd, EventType::Close)) ;
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
}