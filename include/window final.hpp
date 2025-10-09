#pragma once
#include "renderer final.hpp"
#include "event.hpp"

namespace zketch {

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

	class Application {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Window ;

	private :
		static inline std::unordered_map<HWND, Window*> g_windows_ ;
		static inline bool app_is_runing_ = true ;
	
	public :
		static void QuitProgram() noexcept {
			for (auto& w : g_windows_) {
				if (IsWindow(w.first)) {
					DestroyWindow(w.first) ;
				}
			}

			g_windows_.clear() ;
			app_is_runing_ = false ;
			PostQuitMessage(0) ;

			logger::info("Application::QuitProgram - PostQuitMessage done.") ;
		}

		static void QuitWindow(HWND hwnd) noexcept {
			if (hwnd && IsWindow(hwnd)) {
				SendMessage(hwnd, WM_CLOSE, 0, 0) ;

				logger::info("Application::QuitWindow - SendMessage WM_CLOSE for hwnd.") ;
			}
		}

		static bool IsRunning() noexcept {
			return app_is_runing_ ;
		}

	} ;

	namespace AppRegistry {

		static inline HINSTANCE g_hintance_ = GetModuleHandleW(nullptr) ;
		static inline std::string g_window_class_name_ = "zketch_app" ;
		static inline bool window_was_registered = false ;

		static void SetWindowClass(std::string&& windowclassname) noexcept {
			if (!window_was_registered) {
				logger::warning("AppRegistry::SetWindowClass - Failed to register window class name, window class name was registered.") ;
				return ;
			} 

			g_window_class_name_ = std::move(windowclassname) ;
		}

		static void RegisterWindowClass() {
			if (window_was_registered) {
				logger::warning("AppRegistry::RegisterWindowClass - Failed to register window class name, window class name was registered.") ;
				return ;
			}

			WNDCLASSEX wc = {
				sizeof(wc),
				CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
				wndproc,
				0,
				0,
				AppRegistry::g_hintance_,
				LoadIcon(nullptr, IDI_APPLICATION),
				LoadCursor(nullptr, IDC_ARROW),
				nullptr,
				nullptr,
				AppRegistry::g_window_class_name_.c_str(),
				LoadIcon(nullptr, IDI_APPLICATION)
			} ;

			if (!RegisterClassEx(&wc)) {
				logger::error("AppRegistry::RegisterWindowClass - Failed to register window class!") ;
				return ;
			}

			logger::info("AppRegistry::RegisterWindowClass - Successfully register window class.") ;
			window_was_registered = true ;
		}
	} ;

	class Window {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

	private :
		HWND hwnd_ = nullptr ;
		std::unique_ptr<Canvas> canvas_ ;

		void CreateCanvas(const Size& size) noexcept {
			if (!canvas_) {
				canvas_ = std::make_unique<Canvas>() ;
			}

			if (!canvas_->Create(size)) {
				logger::error("Window::CreateCanvas - failed to create back buffer canvas.") ;
				return ;
			}

			logger::info("Window::CreateCanvas - Successfully create with size : [", size.x, "x", size.y, "] .") ;
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
				logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				return ;
			}

			Application::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window::Window - Create Window success.") ;
			canvas_ = std::make_unique<Canvas>() ;
			CreateCanvas(GetClientBound().GetSize()) ;
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
				logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				return ;
			}

			Application::g_windows_.emplace(hwnd_, this) ;
			logger::info("Window::Window - Create Window success.") ;
			canvas_ = std::make_unique<Canvas>() ;
			CreateCanvas(GetClientBound().GetSize()) ;
		}

		Window(Window&& o) noexcept : hwnd_(std::exchange(o.hwnd_, nullptr)), canvas_(std::move(o.canvas_)) {
			logger::info("Window::Window - Calling move ctor.") ;
			if (hwnd_) {
				auto it = Application::g_windows_.find(o.hwnd_) ;
				if (it != Application::g_windows_.end()) 
					Application::g_windows_.erase(it) ;
				Application::g_windows_[hwnd_] = this ;
			}
		}

		~Window() noexcept {
			logger::info("Window::~Window - Calling window dtor") ;
			if (hwnd_) {
				if (IsWindow(hwnd_)) {
					DestroyWindow(hwnd_) ;
				}
				hwnd_ = nullptr ;
				logger::info("Window::~Window - Success to set hwnd to nullptr.") ;
			} else {
				logger::warning("Window::~Window - Failed to set hwnd, hwnd is already nullptr.") ;
			}
		}

		Window& operator=(Window&& o) noexcept {
			logger::info("Calling move assignment.") ;
			if (this != &o) {
				hwnd_ = std::exchange(o.hwnd_, nullptr) ;
				canvas_ = std::move(o.canvas_) ;
				
				auto it = Application::g_windows_.find(o.hwnd_) ;
				if (it != Application::g_windows_.end()) {
					Application::g_windows_.erase(it) ;
				}

				Application::g_windows_[hwnd_] = this ;
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

		void AttachCanvas(const CanvasAttachment& place) noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Window::AttachCanvas - Invalid window canvas") ;
				return ;
			}

			auto* bitmap = place.canvas_->GetBitmap() ;
			if (!bitmap) {
				logger::warning("Window::AttachCanvas - Source canvas has no bitmap") ;
				return ;
			}

			Renderer renderer ;
			if (renderer.Begin(*canvas_)) {
				renderer.DrawCanvas(place.canvas_, place.pos_) ;
				renderer.End() ;
			}
		}

		template <typename ... CanvasAttachments, typename = std::enable_if_t<(std::is_same_v<CanvasAttachments, CanvasAttachment> && ...)>>
		void AttachCanvas(const CanvasAttachment& place, const CanvasAttachments& ... rest) noexcept {
			AttachCanvas(place) ;
			AttachCanvas(rest...) ;
		}

		// Actually display the back buffer to screen
		void Display(Renderer& renderer, bool clear = true) noexcept {
			renderer.Present(canvas_.get(), hwnd_) ;

			if (clear) {
				canvas_->Clear() ;
			}
		}
	} ;

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
			case WM_SIZE : {
				auto it = Application::g_windows_.find(hwnd) ;
				if (it != Application::g_windows_.end()) {
					Application::g_windows_[hwnd]->CreateCanvas({LOWORD(lp), HIWORD(lp)}) ;
				} 
				EventSystem::PushEvent(Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
			}
			case WM_CLOSE : 
				EventSystem::PushEvent(Event::CreateNormalEvent(hwnd, EventType::Close)) ;
				DestroyWindow(hwnd) ;
				return 0 ;

			case WM_DESTROY : 
				if (hwnd) {
					auto it = Application::g_windows_.find(hwnd) ;
					if (it != Application::g_windows_.end()) {
						Application::g_windows_.erase(it) ;
						logger::info("erased window from g_windows_. current size : ", Application::g_windows_.size()) ;
					} else {
						logger::info("WM_DESTROY for hwnd not found in g_windows_. size: ", Application::g_windows_.size()) ;
					}
					logger::info("current g_windows size : ", Application::g_windows_.size()) ;
				}

				if (Application::g_windows_.empty()) {
					Application::QuitProgram() ;
				}

				return 0 ;
		}

		return DefWindowProc(hwnd, msg, wp, lp) ;
	}

}