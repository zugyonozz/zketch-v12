<<<<<<< HEAD
#pragma once
#include "canvas.hpp"
#include "event.hpp"

namespace zketch {

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

	class Application {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Window ;

	private :
		static inline std::unordered_map<HWND, Window*> g_windows_ ;
		static inline bool app_is_runing_ = true ;

		static void RegisterWindow(HWND hwnd, Window* window) noexcept {
			if (hwnd && window) {
				g_windows_[hwnd] = window ;
				
				#ifdef APPLICATION_DEBUG
					logger::info("Application::RegisterWindow - Registered window, current size: ", g_windows_.size()) ;
				#endif

			}
		}

		static void UnRegisterWindow(HWND hwnd) noexcept {
			if (hwnd) {
				auto it = g_windows_.find(hwnd) ;
				if (it != g_windows_.end()) {
					g_windows_.erase(it) ;

					#ifdef APPLICATION_DEBUG
						logger::info("Application::UnRegisterWindow - Erased window from g_windows_, current size: ", g_windows_.size()) ;
					#endif

				} else {

					#ifdef APPLICATION_DEBUG
						logger::info("Application::UnRegisterWindow - hwnd not found in g_windows_. size: ", g_windows_.size()) ;
					#endif

				}
			}
		}
	
	public :
		static void QuitProgram() noexcept {
			std::vector<HWND> destroy_sequence ;
			destroy_sequence.reserve(g_windows_.size()) ;
			for (auto& w : g_windows_) {
				destroy_sequence.push_back(w.first) ;
			}

			for (auto& w : destroy_sequence) {
				if (IsWindow(w)) {
					DestroyWindow(w) ;
				}
			}

			g_windows_.clear() ;
			app_is_runing_ = false ;
			PostQuitMessage(0) ;

			#ifdef APPLICATION_DEBUG
				logger::info("Application::QuitProgram - PostQuitMessage done.") ;
			#endif
		}

		static bool IsRunning() noexcept {
			return app_is_runing_ ;
		}

		static bool LoadFonts() noexcept {
			auto fontMapOpt = ___FONT_DUMP___::__font_dump__::LoadFontsFromBin("fonts.bin") ;
			
			if (!fontMapOpt) {
				#ifdef APPLICATION_DEBUG
					logger::error("Application::LoadFonts - Failed load fonts from fonts.bin"); 
					logger::info("Application::LoadFonts - Trying to create dump fonts."); 
				#endif

				// Create dump if not exists
				if (!___FONT_DUMP___::__font_dump__::CreateDumpFonts("fonts", "")) {
					#ifdef APPLICATION_DEBUG
						logger::error("Application::LoadFonts - Failed to create dump fonts.") ; 
					#endif
					return false ;
				}

				#ifdef APPLICATION_DEBUG
					logger::info("Application::LoadFonts - Successfully create dump font.") ;
					logger::info("Application::LoadFonts - Trying to load fonts again.") ;
				#endif

				// Try load again
				fontMapOpt = ___FONT_DUMP___::__font_dump__::LoadFontsFromBin("fonts.bin") ;

				if (!fontMapOpt) {
					#ifdef APPLICATION_DEBUG
						logger::error("Application::LoadFonts - Failed load fonts after dump creation.") ;
					#endif
					return false ;
				}
			}

			Font::g_fonts_ = std::move(*fontMapOpt) ;

			#ifdef APPLICATION_DEBUG
				logger::info("Application::LoadFonts - Successfully loaded ", Font::g_fonts_.size(), " fonts.") ;
			#endif

			return !Font::g_fonts_.empty() ;
		}

		static size_t GetFontCount() noexcept {
			return Font::g_fonts_.size() ;
		}

		static std::vector<std::string> GetAvailableFonts() noexcept {
			std::vector<std::string> fonts ;
			std::unordered_set<std::string> unique_names ;
			
			for (const auto& [key, data] : Font::g_fonts_) {
				std::string_view name_view(data.fontname_.data(), strlen(data.fontname_.data())) ;
				std::string name(name_view) ;
				if (unique_names.insert(name).second) {
					fonts.push_back(std::move(name)) ;
				}
			}
			
			std::sort(fonts.begin(), fonts.end()) ;
			return fonts ;
		}

		// Check apakah font dengan style tertentu tersedia
		static bool IsFontAvailable(const std::string_view& fontname, FontStyle style = FontStyle::Regular) noexcept {
			return ___FONT_DUMP___::__font_dump__::___find_font___(Font::g_fonts_, fontname, static_cast<uint8_t>(style)) != nullptr ;
		}
	} ;

	namespace AppRegistry {
		static inline HINSTANCE g_hinstance_ = GetModuleHandleW(nullptr) ;
		static inline std::string g_window_class_name_ = "zketch_app" ;
		static inline bool window_was_registered = false ;

		static void SetWindowClass(std::string&& windowclassname) noexcept {
			if (window_was_registered) {

				#ifdef APPREGISTRY_DEBUG
					logger::warning("AppRegistry::SetWindowClass - Failed to register window class name, window class name was registered.") ;
				#endif

				return ;
			} 
			g_window_class_name_ = std::move(windowclassname) ;
		}

		static void RegisterWindowClass() {
			if (window_was_registered) {

				#ifdef APPREGISTRY_DEBUG
					logger::warning("AppRegistry::RegisterWindowClass - Failed to register window class name, window class name was registered.") ;
				#endif

				return ;
			}

			WNDCLASSEX wc = {
				sizeof(wc),
				CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
				wndproc,
				0,
				0,
				AppRegistry::g_hinstance_,
				LoadIcon(nullptr, IDI_APPLICATION),
				LoadCursor(nullptr, IDC_ARROW),
				nullptr,
				nullptr,
				AppRegistry::g_window_class_name_.c_str(),
				LoadIcon(nullptr, IDI_APPLICATION)
			} ;

			if (!RegisterClassEx(&wc)) {

				#ifdef APPREGISTRY_DEBUG
					logger::error("AppRegistry::RegisterWindowClass - Failed to register window class!") ;
				#endif

				return ;
			}

			#ifdef APPREGISTRY_DEBUG
				logger::info("AppRegistry::RegisterWindowClass - Successfully register window class.") ;
			#endif

			window_was_registered = true ;
		}
	} ;

	class Window {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Renderer ;

	private :
		HWND handle_ = nullptr ;
		std::unique_ptr<Canvas> front_buffer_ ;
		std::unique_ptr<Canvas> back_buffer_ ;
		WindowState state_ = WindowState::None ;
		bool close_requested_ = false ;

		void CreateCanvas(const Size& size) noexcept {
			if ((state_ & WindowState::Destroyed) != WindowState::Destroyed) {
				if (!front_buffer_) {
					front_buffer_ = std::make_unique<Canvas>() ;
				}

				if (!back_buffer_) {
					back_buffer_ = std::make_unique<Canvas>() ;
				}

				if (!front_buffer_->Create(size)) {
					#ifdef WINDOW_DEBUG
						logger::error("Window::CreateCanvas - failed to create front buffer canvas.") ;
					#endif
					return ;
				}

				if (!back_buffer_->Create(size)) {
					#ifdef WINDOW_DEBUG
						logger::error("Window::CreateCanvas - failed to create back buffer canvas.") ;
					#endif
					return ;
				}

				#ifdef WINDOW_DEBUG
					logger::info("Window::CreateCanvas - Successfully create with size: [", size.x, "x", size.y, "].") ;
				#endif
			}
		}

		bool IsCanvasValid() const noexcept {
    		return front_buffer_ && back_buffer_ && front_buffer_->IsValid() && back_buffer_->IsValid() && ((state_ & WindowState::Destroyed) != WindowState::Destroyed) ;
		}

		void InternalDestroy() noexcept {
			if ((state_ & WindowState::Destroyed) == WindowState::Destroyed) {
				return ;
			}

			#ifdef WINDOW_DEBUG
				logger::info("Window::InternalDestroy - Starting destruction process") ;
			#endif

			// Unregister dari Application
			if ((state_ & WindowState::Register) == WindowState::Register) {
				Application::UnRegisterWindow(handle_) ;
				state_ &= ~WindowState::Register ;
				state_ |= WindowState::UnRegister ;
			}

			// Destroy window handle
			if (handle_ && IsWindow(handle_)) {
				DestroyWindow(handle_) ;
			}
			
			handle_ = nullptr ;
			state_ |= WindowState::Destroyed ;

			// Clear canvases
			front_buffer_.reset() ;
			back_buffer_.reset() ;

			#ifdef WINDOW_DEBUG
				logger::info("Window::InternalDestroy - Destruction complete") ;
			#endif
		}

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, int32_t width, int32_t height) noexcept {
			handle_ = CreateWindowEx(
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
				AppRegistry::g_hinstance_,
				nullptr
			) ;

			if (!handle_) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				#endif

				return ;
			}

			state_ = WindowState::Active ;
			Application::RegisterWindow(handle_, this) ;
			state_ |= WindowState::Register ;

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Create Window success.") ;
			#endif

			CreateCanvas(GetClientBound().GetSize()) ;
		}

		Window(const char* title, int32_t xpos, int32_t ypos, int32_t width, int32_t height) noexcept {
			handle_ = CreateWindowEx(
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
				AppRegistry::g_hinstance_,
				nullptr
			) ;

			if (!handle_) {
				#ifdef WINDOW_DEBUG
					logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				#endif
				return ;
			}

			state_ = WindowState::Active ;
			Application::RegisterWindow(handle_, this) ;
			state_ |= WindowState::Register ;

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Create Window success.") ;
			#endif

			CreateCanvas(GetClientBound().GetSize()) ;
		}

		Window(Window&& o) noexcept : 
		handle_(std::exchange(o.handle_, nullptr)),
		front_buffer_(std::move(o.front_buffer_)), 
		back_buffer_(std::move(o.back_buffer_)),
		state_(std::exchange(o.state_, WindowState::None)),
		close_requested_(std::exchange(o.close_requested_, false)) {

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Calling move ctor.") ;
			#endif

			if (handle_) {
				Application::UnRegisterWindow(handle_) ;
				Application::RegisterWindow(handle_, this) ;
			}
		}

		~Window() noexcept {
			#ifdef WINDOW_DEBUG
				logger::info("Window::~Window - Calling window dtor") ;
			#endif

			InternalDestroy() ;
		}

		Window& operator=(Window&& o) noexcept {
			#ifdef WINDOW_DEBUG
				logger::info("Window::operator= - Calling move assignment.") ;
			#endif

			if (this != &o) {
				InternalDestroy() ;

				handle_ = std::exchange(o.handle_, nullptr) ;
				front_buffer_ = std::move(o.front_buffer_) ;
				back_buffer_ = std::move(o.back_buffer_) ;
				state_ = std::exchange(o.state_, WindowState::None) ;
				close_requested_ = std::exchange(o.close_requested_, false) ;

				if (handle_) {
					Application::UnRegisterWindow(handle_) ;
					Application::RegisterWindow(handle_, this) ;
				}
			}

			return *this ;
		}

		void Show() const noexcept {
			if (handle_) {
				ShowWindow(handle_, SW_SHOWDEFAULT) ;
				UpdateWindow(handle_) ;
			}
		}

		void Hide() const noexcept {
			if (handle_) {
				ShowWindow(handle_, SW_HIDE) ; 
			}
		}

		void Minimize() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_MINIMIZE) ; 
			}
		}

		void Maximize() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_MAXIMIZE) ; 
			}
		}

		void Restore() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_RESTORE) ; 
			}
		}

		void Close() noexcept {

			#ifdef WINDOW_DEBUG
				logger::info("Window::Close - Close requested for window: ", handle_) ;
			#endif

			InternalDestroy() ;
		}

		void Present() const noexcept {
			if (!front_buffer_ || !front_buffer_->IsValid()) {

				#ifdef WINDOW_DEBUG
					logger::warning("Window::Present - Invalid canvas!") ;
				#endif

				return ;
			}

			HDC hdc = GetDC(handle_) ;
			if (!hdc) {

				#ifdef WINDOW_DEBUG
					logger::warning("Window::Present - Invalid HDC!") ;
				#endif
				
				return ;
			}

			Gdiplus::Graphics screen(hdc) ;
			if (screen.GetLastStatus() != Gdiplus::Ok) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::Present - Graphics creation failed");
				#endif

				ReleaseDC(handle_, hdc);
				return;
			}

			screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			auto status = screen.DrawImage(front_buffer_->GetBitmap(), 0, 0) ;

			if (status != Gdiplus::Ok) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::Present - DrawImage failed: ", static_cast<int>(status));
				#endif
				
			}

			ReleaseDC(handle_, hdc) ;
		}

		void SetTitle(const char* title) noexcept {
			if (handle_) {
				SetWindowText(handle_, title) ;
			}
		}

		Rect GetClientBound() const noexcept {
			tagRECT r ;
			GetClientRect(handle_, &r) ;
			return static_cast<Rect>(r) ;
		}

		Rect GetWindowBound() const noexcept {
			tagRECT r ;
			GetWindowRect(handle_, &r) ;
			return static_cast<Rect>(r) ; 
		}

		HWND GetHandle() const noexcept { return handle_ ; }
		bool IsWindowValid() const noexcept { return handle_ && (state_ & WindowState::Destroyed) != WindowState::Destroyed ; }
		bool IsCloseRequested() const noexcept { return close_requested_ ; }
	} ;

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
			case WM_SIZE : {
				auto it = Application::g_windows_.find(hwnd) ;
				if (it != Application::g_windows_.end()) {
					it->second->CreateCanvas({LOWORD(lp), HIWORD(lp)}) ;
				} 
				EventSystem::PushEvent(Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
			}

			case WM_CLOSE : {
				EventSystem::PushEvent(Event::CreateCommonEvent(hwnd, EventType::Close)) ;
				
				auto it = Application::g_windows_.find(hwnd) ;
				if (it != Application::g_windows_.end()) {
					it->second->close_requested_ = true ;
				}
				
				return 0 ;
			}

			case WM_DESTROY : {
				Application::UnRegisterWindow(hwnd) ;
				if (Application::g_windows_.empty()) {
					PostQuitMessage(0) ;
					Application::app_is_runing_ = false ;
					
					#ifdef WINDOW_DEBUG
						logger::info("wndproc - All windows closed, posting quit message.") ;
					#endif
				}
				return 0 ;
			}
		}

		return DefWindowProc(hwnd, msg, wp, lp) ;
	}

=======
#pragma once

#include "renderer.hpp"
#include "event.hpp"

namespace zketch {

	class Window ;

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
			case WM_SIZE : 
				EventSystem::PushEvent(Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
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
	} ;
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}