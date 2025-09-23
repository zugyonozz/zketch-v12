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
			Color background = rgba(240, 240, 240, 255) ;     // Light gray background
			Color track_fill = rgba(200, 200, 200, 255) ;     // Track color
			Color track_stroke = rgba(150, 150, 150, 255) ;   // Track border
			Color thumb_fill = rgba(100, 149, 237, 255) ;     // Cornflower blue thumb
			Color thumb_stroke = rgba(70, 130, 180, 255) ;    // Steel blue thumb border
			Color thumb_hover = rgba(135, 206, 250, 255) ;    // Light sky blue on hover
			float track_thickness = 6.0f ;    // Track thickness
			float thumb_size = 16.0f ;        // Thumb size
			float thumb_thickness = 1.0f ;   // Border thickness
			float corner_radius = 3.0f ;      // Rounded corners
		} ;

	private :
		Orientation orientation_ ;
		RectF bounds_ ;
		float min_value_ ;
		float max_value_ ;
		float current_value_ ;
		bool is_dragging_ ;
		bool is_hover_ ;
		bool is_update_ = true ;
		Style style_ ;
		std::unique_ptr<Canvas> canvas_ ;
		std::unique_ptr<Drawer> drawer_ ;

		// Calculate thumb position based on current value
		PointF GetThumbPosition() const noexcept {
			float ratio = (current_value_ - min_value_) / (max_value_ - min_value_);
			ratio = std::clamp(ratio, 0.0f, 1.0f );

			if (orientation_ == Horizontal) {
				float track_width = bounds_.w - style_.thumb_size ;
				float thumb_x = bounds_.x + (track_width * ratio) ;
				float thumb_y = bounds_.y + (bounds_.h - style_.thumb_size) / 2.0f ;
				return {thumb_x, thumb_y} ;
			} else {
				float track_height = bounds_.h - style_.thumb_size ;
				float thumb_x = bounds_.x + (bounds_.w - style_.thumb_size) / 2.0f ;
				// Invert for vertical (top = max, bottom = min)
				float thumb_y = bounds_.y + track_height * (1.0f - ratio) ;
				return {thumb_x, thumb_y} ;
			}
		}

		// Calculate value from mouse position
		float GetValueFromPosition(const PointF& pos) const noexcept {
			float ratio ;
			if (orientation_ == Horizontal) {
				float track_width = bounds_.w - style_.thumb_size ;
				float relative_pos = pos.x - bounds_.x - style_.thumb_size / 2.0f ;
				ratio = relative_pos / track_width ;
			} else {
				float track_height = bounds_.h - style_.thumb_size ;
				float relative_pos = pos.y - bounds_.y - style_.thumb_size / 2.0f ;
				// Invert for vertical
				ratio = 1.0f - (relative_pos / track_height) ;
			}
			
			ratio = std::clamp(ratio, 0.0f, 1.0f);
			return min_value_ + ratio * (max_value_ - min_value_);
		}

		// Get thumb bounds for hit testing
		RectF GetThumbBounds() const noexcept {
			PointF thumb_pos = GetThumbPosition();
			return {thumb_pos.x, thumb_pos.y, style_.thumb_size, style_.thumb_size};
		}

		// Get track bounds
		RectF GetTrackBounds() const noexcept {
			if (orientation_ == Horizontal) {
				float track_y = bounds_.y + (bounds_.h - style_.track_thickness) / 2.0f ;
				return {bounds_.x + style_.thumb_size / 2.0f, track_y, bounds_.w - style_.thumb_size, style_.track_thickness} ;
			} else {
				float track_x = bounds_.x + (bounds_.w - style_.track_thickness) / 2.0f ;
				return {track_x, bounds_.y + style_.thumb_size / 2.0f, style_.track_thickness, bounds_.h - style_.thumb_size} ;
			}
		}

		void Update() noexcept {
			if (!is_update_) {
				return ;
			}

			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Slider::UpdateCanvas - Canvas not valid.") ;
				return ;
			}

			if (!drawer_->Begin(*canvas_)) {
				logger::warning("Slider::UpdateCanvas - Failed to begin drawing") ;
				return ;
			}

			drawer_->Clear(style_.background) ;
			RectF track_bound = GetTrackBounds() ;
			drawer_->FillRect(track_bound, style_.track_fill) ;
			drawer_->DrawRect(track_bound, style_.track_stroke, style_.track_thickness) ;

			RectF thumb_bound = GetTrackBounds() ;
			Color thumb_color = is_hover_ ? style_.thumb_hover : style_.thumb_fill ;
			drawer_->FillRect(thumb_bound, style_.thumb_fill) ;
			drawer_->DrawRect(thumb_bound, style_.thumb_stroke, style_.thumb_thickness) ;

			drawer_->End() ;
			is_update_ = false ;
		}

	public :
		Slider(Orientation orientation, const RectF& bounds, 
			   float min_val = 0.0f, float max_val = 100.0f, float initial_val = 0.0f) noexcept
			: orientation_(orientation), bounds_(bounds), 
			  min_value_(min_val), max_value_(max_val), 
			  current_value_(std::clamp(initial_val, min_val, max_val)),
			  is_dragging_(false), is_hover_(false), is_update_(true) {
				canvas_ = std::make_unique<Canvas>() ;
				drawer_ = std::make_unique<Drawer>() ;

				Point canvas_size(static_cast<int32_t>(bounds_.w), static_cast<int32_t>(bounds_.h)) ;
				if (!canvas_->Create(canvas_size)) {
					logger::error("Failed to create slider canvas") ;
					return ;
				}

				Update() ;
		}

		~Slider() noexcept = default ;

		// Handle mouse events
		bool OnMouseDown(const PointF& pos) noexcept {
			RectF thumb_bounds = GetThumbBounds() ;
			if (thumb_bounds.Contain(pos)) {
				is_dragging_ = true ;
				is_update_ = true ;
				EventSystem::PushEvent(Event::createSliderEvent(nullptr, SliderEventType::DragStart, current_value_, this)) ;
				Update() ;
				return true ; // Event handled
			}
			
			// Click on track to jump to position
			RectF track_bounds = GetTrackBounds() ;
			if (track_bounds.Contain(pos)) {
				float new_value = GetValueFromPosition(pos) ;
				if (std::abs(new_value - current_value_) > 0.001f) {
					current_value_ = new_value ;
					is_update_ = true ;
					EventSystem::PushEvent(Event::createSliderEvent(nullptr, SliderEventType::ValueChanged, current_value_, this)) ;
					Update() ;
				}
				return true ; // Event handled
			}
			
			return false ; // Event not handled
		}

		bool OnMouseMove(const PointF& pos) noexcept {
			bool was_hover = is_hover_ ;
			RectF thumb_bounds = GetThumbBounds() ;
			is_hover_ = thumb_bounds.Contain(pos) ;
			
			if (is_dragging_) {
				float new_value = GetValueFromPosition(pos) ;
				if (std::abs(new_value - current_value_) > 0.001f) {
					current_value_ = new_value ;
					is_update_ = true ;
					EventSystem::PushEvent(Event::createSliderEvent(nullptr, SliderEventType::ValueChanged, current_value_, this)) ;
					Update() ;
				}
				return true ;  // Event handled
			}
			
			// Redraw if hover state changed
			if (was_hover != is_hover_) {
				is_update_ = true ;
				Update() ;
				return true ;
			}
			
			return false ;
		}

		bool OnMouseUp(const PointF& pos) noexcept {
			if (is_dragging_) {
				is_dragging_ = false ;
				is_update_ = true ;
				EventSystem::PushEvent(Event::createSliderEvent(nullptr, SliderEventType::DragEnd, current_value_, this)) ;
				Update() ;
				return true ; // Event handled
			}
			return false ; // Event not handled
		}

		// Getters and Setters
		float GetValue() const noexcept { return current_value_ ; }
		
		void SetValue(float value) noexcept {
			float new_value = std::clamp(value, min_value_, max_value_) ;
			if (new_value != current_value_) {
				current_value_ = new_value ;
				EventSystem::PushEvent(Event::createSliderEvent(nullptr, SliderEventType::ValueChanged, current_value_, this)) ;
			}
		}

		float GetMin() const noexcept { return min_value_ ; }
		float GetMax() const noexcept { return max_value_ ; }

		void SetRange(float min_val, float max_val) noexcept {
			min_value_ = min_val ;
			max_value_ = max_val ;
			current_value_ = std::clamp(current_value_, min_value_, max_value_) ;
		}

		const RectF& GetBounds() const noexcept { return bounds_ ; }

		void SetBounds(const RectF& bounds) noexcept { 
			bounds_ = bounds ; 
			is_update_ = true ;

			if (canvas_ && !canvas_->Create(bounds.getSize())) {
				logger::error("Failed to recreate slider canvas") ;
			}
		}

		Style& GetStyle() noexcept { return style_ ; }
		const Style& GetStyle() const noexcept { return style_ ; }

		bool IsDragging() const noexcept { return is_dragging_ ; }
		bool IsHover() const noexcept { return is_hover_ ; }
		
		Orientation GetOrientation() const noexcept { return orientation_; }

		void Present(HWND hwnd) noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Slider::Present - Canvas not valid") ;
				return ;
			}

			Update() ;

			canvas_->Present(hwnd, {static_cast<int32_t>(bounds_.x), static_cast<int32_t>(bounds_.y)}) ;
		}
	} ;
}