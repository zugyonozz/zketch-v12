#pragma once
#include "unit.hpp"

namespace zketch {

	namespace error_handler {
		struct invalid_event_type {
			const char* what() const noexcept {
				return "invalid event type" ;
			}
		} ;
	}

	class Event {
		friend inline bool PollEvent(Event&) ;

	private :
		EventType type_ = EventType::None ;
		HWND hwnd_ = nullptr ;
		uint64_t timestamp_ = 0 ;

		union data_ {
			struct empty__ {} empty_ ;

			struct Key__ {
				KeyState state_ ;
				uint32_t key_code_ ;
			} key_ ;

			struct Mouse__ {
				MouseState state_ ;
				MouseButton button_ ;
				int32_t x_ ;
				int32_t y_ ;
				int32_t value_ ;
			} mouse_ ;

			struct Resize__ {
				uint32_t width_ ;
				uint32_t height_ ;
			} resize_ ;

			struct Slider__ {
				SliderState state_ ;
				float value_ ;
				Slider* slider_ptr_ ;
			} slider_ ;

			struct Button__ {
				ButtonState state_ ;
				Button* button_ptr_ ;
			} button_ ;
		} data_ ;

		// -------------- Construtor  --------------

		constexpr Event(HWND src_, EventType type_) {
			this->type_ = type_ ;
			if ((IsMouseEvent() || IsKeyEvent() || IsResizeEvent() || IsSliderEvent())) {
				throw error_handler::invalid_event_type() ;
			}
			data_.empty_ = {} ;
			hwnd_ = src_ ;
		}

		constexpr Event(HWND src, const Size& size) noexcept {
			type_ = EventType::Resize ;
			data_.resize_ = {size.x, size.y} ;
			hwnd_ = src ;
		}

		constexpr Event(HWND src, KeyState state, uint32_t key_code) {
			type_ = EventType::Key ;
			if (!IsKeyEvent()) {
				throw error_handler::invalid_event_type() ;
			}

			data_.key_.state_ = state ;
			data_.key_.key_code_ = key_code ;
			hwnd_ = src ;
		}

		constexpr Event(HWND src, MouseButton button, MouseState state, const Point& pos, int32_t value = 0) {
			type_ = EventType::Mouse ;
			if (!IsMouseEvent()) {
				throw error_handler::invalid_event_type() ;
			}
		
			if (state == MouseState::None) {
				data_.mouse_ = {
					state, 
					MouseButton::None,
					pos.x, 
					pos.y, 
					0
				} ;
			} else if (state == MouseState::Wheel) {
				data_.mouse_ = {
					state, 
					MouseButton::None,
					pos.x, 
					pos.y, 
					value
				} ;
			} else {
				data_.mouse_ = {
					state, 
					button, 
					pos.x, 
					pos.y,
					value
				} ;
			}
			hwnd_ = src ;
		}

		constexpr Event(SliderState state, float value, Slider* slider_ptr = nullptr) noexcept {
			type_ = EventType::Slider ;
			data_.slider_ = {
				state, 
				value, 
				slider_ptr
			} ;
			hwnd_ = nullptr ;
		}

		constexpr Event(ButtonState state, Button* button_ptr = nullptr) noexcept {
			type_ = EventType::Button ;
			data_.button_ = {
				state,
				button_ptr
			} ;
		}

		static constexpr Event CreateEventFromMSG(const MSG& msg) noexcept {
			switch (msg.message) {
				case WM_KEYDOWN : 
					return Event::CreateKeyEvent(msg.hwnd, KeyState::Down, msg.wParam) ; 
				case WM_KEYUP : 
					return Event::CreateKeyEvent(msg.hwnd, KeyState::Up, msg.wParam) ; 
				case WM_MOUSEWHEEL :
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::None, MouseState::Wheel, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}, GET_WHEEL_DELTA_WPARAM(msg.wParam)) ;
				case WM_MOUSEMOVE : 
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::None, MouseState::None, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONDOWN : 
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Left, MouseState::Down, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONDOWN : 
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Right, MouseState::Down, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_MBUTTONDOWN :
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Middle, MouseState::Down, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONUP : 
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Left, MouseState::Up, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONUP : 
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Right, MouseState::Up, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_MBUTTONUP :
					return Event::CreateMouseEvent(msg.hwnd, MouseButton::Middle, MouseState::Up, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_QUIT : 
					return Event::CreateCommonEvent(nullptr, EventType::Quit) ;
				case WM_CLOSE : 
					return Event::CreateCommonEvent(msg.hwnd, EventType::Close) ;
			}
			return Event::CreateCommonEvent(msg.hwnd, EventType::None) ;
		}

	public :
		constexpr Event() noexcept : type_(EventType::None), hwnd_(nullptr) {
			data_.empty_ = {} ;
		}

		static constexpr Event CreateCommonEvent() noexcept {
			return Event(nullptr, EventType::None) ;
		}

		static constexpr Event CreateCommonEvent(HWND src, EventType type) noexcept {
			return Event(src, type) ;
		}

		static constexpr Event CreateKeyEvent(HWND src, KeyState state, uint32_t key_code) noexcept {
			return Event(src, state, key_code) ;
		}

		static constexpr Event CreateMouseEvent(HWND src, MouseButton button, MouseState state, const Point& pos, int32_t value = 0) noexcept {
			return Event(src, button, state, pos, value) ;
		}

		static constexpr Event CreateResizeEvent(HWND src, const Point& size) noexcept {
			return Event(src, size) ;
		}

		static constexpr Event CreateSliderEvent(SliderState state, float value, Slider* slider_ptr = nullptr) {
			return Event(state, value, slider_ptr) ;
		}

		static constexpr Event CreateButtonEvent(ButtonState state, Button* button_ptr = nullptr) {
			return Event(state, button_ptr) ;
		}

		// --------------------------- Getter ---------------------------

		constexpr uint32_t GetKeyCode() const noexcept { 
			return data_.key_.key_code_ ; 
		}

		constexpr KeyState GetKeyState() const noexcept {
			return data_.key_.state_ ;
		}

		constexpr MouseState GetMouseState() const noexcept {
			return data_.mouse_.state_ ;
		}

		constexpr MouseButton GetMouseButton() const noexcept {
			return data_.mouse_.button_ ;
		}

		constexpr int32_t GetMouseWheelValue() const noexcept {
			return data_.mouse_.value_ ;
		}

		constexpr Point GetMousePosition() const noexcept {
			return {data_.mouse_.x_, data_.mouse_.y_} ;
		}

		constexpr Point_<uint32_t> GetResizedSize() const noexcept { 
			return {data_.resize_.width_, data_.resize_.height_} ; 
		}

		constexpr SliderState GetSliderState() const noexcept {
			return data_.slider_.state_ ;
		}

		constexpr float GetSliderValue() const noexcept {
			return data_.slider_.value_ ;
		}

		constexpr Slider* GetSliderAddress() const noexcept {
			return data_.slider_.slider_ptr_ ;
		}

		constexpr ButtonState GetButtonState() const noexcept {
			return data_.button_.state_ ;
		}

		constexpr Button* GetButtonAddress() const noexcept {
			return data_.button_.button_ptr_ ;
		}

		constexpr operator EventType() const noexcept {
			return type_ ;
		}

		constexpr EventType GetEventType() const noexcept {
			return type_ ;
		}

		uint64_t GetTimeStamp() const noexcept {
			return timestamp_ ;
		}

		HWND GetHandle() const noexcept {
			return hwnd_ ;
		}

		// --------------------------- State queries ---------------------------

		constexpr bool IsKeyEvent() const noexcept {
			return (type_ == EventType::Key) ;
		}

		constexpr bool IsMouseEvent() const noexcept {
			return (type_ == EventType::Mouse) ;
		}

		constexpr bool IsResizeEvent() const noexcept {
			return (type_ == EventType::Resize) ;
		}

		constexpr bool IsSliderEvent() const noexcept {
			return (type_ == EventType::Slider) ;
		}

		constexpr bool IsButtonEvent() const noexcept {
			return (type_ == EventType::Button) ;
		}
	} ;

	class EventSystem {
	private :
		static inline std::queue<Event> g_events_ ;
		static inline bool event_was_initialized_ = false ;

	public :
		EventSystem() = delete ;
		EventSystem(const EventSystem&) = delete ;
		EventSystem& operator=(const EventSystem&) = delete ;
		EventSystem(EventSystem&&) = delete ;
		EventSystem& operator=(EventSystem&&) = delete ;

		static void Init() noexcept {
			if (!event_was_initialized_) {
				event_was_initialized_ = true ;

				#ifdef EVENTSYSTEM_DEBUG
					logger::info("EventSystem::Initialize - Initialized!.") ;
				#endif

				return ;
			}
			logger::info("EventSystem::Initialize - Event system was initialized.") ;
		}

		static void PushEvent(const Event& e) noexcept {
			g_events_.push(e) ;
		}

		static bool PollEvent(Event& e) noexcept {
			if (g_events_.empty()) { 

				#ifdef EVENTSYSTEM_DEBUG
					logger::info("EventSystem::PollEvent - Event is empty.") ;
				#endif

				return false ; 
			}

            e = g_events_.front() ;
            g_events_.pop() ;
            return true ;
		}

		static bool PeekEvent(Event& e) noexcept {
			if (g_events_.empty()) {
				return false ;
			}

			e = g_events_.front() ;
			return true ;
		}

		static void Clear() noexcept {
			while (!g_events_.empty()) {
				g_events_.pop() ;
			}

			#ifdef EVENTSYSTEM_DEBUG
				logger::info("EventSystem::Clear - Event cleared!") ;
			#endif

		}
	} ;

	constexpr std::string DescribeKeyState(KeyState state) noexcept {
		switch (state) {
			case KeyState::None : return "None" ;
			case KeyState::Up : return "Up" ;
			case KeyState::Down : return "Down" ;
			default : return "Undefined" ;
		}
		return "Undefined" ;
	}

	constexpr std::string DescribeMouseState(MouseState state) noexcept {
		switch (state) {
			case MouseState::None : return "None" ;
			case MouseState::Up : return "Up" ;
			case MouseState::Down : return "Down" ;
			case MouseState::Wheel : return "Wheel" ;
			default : return "Undefined" ;
		}
		return "Undefined" ;
	}

	constexpr std::string DescribeMouseButton(MouseButton button) noexcept {
		switch (button) {
			case MouseButton::None : return "None" ;
			case MouseButton::Left : return "Left" ;
			case MouseButton::Right : return "Right" ;
			case MouseButton::Middle : return "Middle" ;
			default : return "Undefined" ;
		}
		return "Undefined" ;
	}

	constexpr std::string DescribeSliderState(SliderState state) noexcept {
		switch (state) {
			case SliderState::None : return "None" ;
			case SliderState::Start : return "Start" ;
			case SliderState::End : return "End" ;
			case SliderState::Changed : return "Changed" ;
			case SliderState::Hover : return "Hover" ;
			default : return "Undefined" ;
		}
		return "Undefined" ;
	}

	constexpr std::string DescribeButtonState(ButtonState state) noexcept {
		switch (state) {
			case ButtonState::None : return "None" ;
			case ButtonState::Press : return "Press" ;
			case ButtonState::Release : return "Release" ;
			case ButtonState::Hover : return "Hover" ;
			default : return "Undefined" ;
		}
		return "Undefined" ;
	}

	inline bool PollEvent(Event& e) {
		if (EventSystem::PollEvent(e)) {
			return true ;
		}

		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				
				#ifdef POLLEVENT_DEBUG
					logger::info("PollEvent - WM_QUIT received via PeekMessage.") ;
				#endif

				e = Event::CreateCommonEvent(nullptr, EventType::Quit) ;
				return true ;
			}

			Event ecvt = Event::CreateEventFromMSG(msg) ;

			if (ecvt != EventType::None) {
				EventSystem::PushEvent(ecvt) ;
			}

			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
		}

		return EventSystem::PollEvent(e) ;
	}

}