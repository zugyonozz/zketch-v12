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

	enum class EventType : uint8_t { 
		None,
		Quit,
		Close,
		Key,
		Mouse,
		Resize, 
		Slider,
		Button
	} ;

	enum class MouseState : uint8_t {
		None,
		Up,
		Down,
		Wheel
	} ;

	enum class MouseButton : uint8_t { 
		None,
		Left, 
		Right, 
		Middle, 
	} ;

	enum class KeyState : uint8_t {
		None,
		Up,
		Down
	} ;

	enum class SliderState : uint8_t {
		None,
		Changed,
		Start,
		End,
		Hover
	} ;

	enum class ButtonState : uint8_t {
		None,
		Hover,
		Press,
		Release
	} ;

	struct Key__ {
		KeyState state_ ;
		uint32_t key_code_ ;
	} ;

	struct Slider__ {
		SliderState state_ ;
		float value_ ;
		void* slider_ptr_ ;
	} ;

	struct Button__ {
		ButtonState state_ ;
		void* button_ptr_ ;
	} ;

	struct Mouse__ {
		MouseState state_ ;
		MouseButton button_ ;
		int32_t x_ ;
		int32_t y_ ;
		int32_t value_ ;
	} ;

	struct Resize__ {
		uint32_t width_ ;
		uint32_t height_ ;
	} ;

	class Event {
	private :
		EventType type_ = EventType::None ;
		HWND hwnd_ = nullptr ;
		uint64_t timestamp_ = 0 ;

		union data_ {
			struct empty__ {} empty_ ;
			Key__ key_ ;
			Mouse__ mouse_ ;
			Resize__ resize_ ;
			Slider__ slider_ ;
			Button__ button_ ;
		} data_ ;

		// -------------- Construtor  --------------

		constexpr Event(HWND src, KeyState state, uint32_t key_code) {
			type_ = EventType::Key ;
			if (!IsKeyEvent()) {
				throw error_handler::invalid_event_type() ;
			}

			data_.key_.state_ = state ;
			data_.key_.key_code_ = key_code ;
			hwnd_ = src ;
		}

		constexpr Event(HWND src, const Size& size) noexcept {
			type_ = EventType::Resize ;
			data_.resize_ = {size.x, size.y} ;
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
					MouseButton::None, // ignore to MouseButton::None
					pos.x, 
					pos.y, 
					0
				} ;
			} else if (state == MouseState::Wheel) {
				data_.mouse_ = {
					state, 
					MouseButton::None, // ignore to MouseButton::None
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

		constexpr Event(SliderState state, float value, void* slider_ptr = nullptr) noexcept {
			type_ = EventType::Slider ;
			data_.slider_ = {
				state, 
				value, 
				slider_ptr
			} ;
			hwnd_ = nullptr ;
		}

		Event(ButtonState state, void* button_ptr = nullptr) noexcept {
			type_ = EventType::Button ;
			data_.button_ = {
				state,
				button_ptr
			} ;
		}

		constexpr Event(HWND src_, EventType type_) {
			this->type_ = type_ ;
			if ((IsMouseEvent() || IsKeyEvent() || IsResizeEvent() || IsSliderEvent())) {
				throw error_handler::invalid_event_type() ;
			}
			data_.empty_ = {} ;
			hwnd_ = src_ ;
		}

	public :
		constexpr Event() noexcept : type_(EventType::None), hwnd_(nullptr) {
			data_.empty_ = {} ;
		}

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

		static Event CreateNormalEvent() noexcept {
			return Event(nullptr, EventType::None) ;
		}

		static Event CreateNormalEvent(HWND src, EventType type) noexcept {
			return Event(src, type) ;
		}

		static Event CreateKeyEvent(HWND src, KeyState state, uint32_t key_code) noexcept {
			return Event(src, state, key_code) ;
		}

		static Event CreateMouseEvent(HWND src, MouseButton button, MouseState state, const Point& pos, int32_t value = 0) noexcept {
			return Event(src, button, state, pos, value) ;
		}

		static Event CreateResizeEvent(HWND src, const Point& size) noexcept {
			return Event(src, size) ;
		}

		static Event CreateSliderEvent(SliderState state, float value, void* slider_ptr = nullptr) {
			return Event(state, value, slider_ptr) ;
		}

		static Event CreateButtonEvent(ButtonState state, void* button_ptr = nullptr) {
			return Event(state, button_ptr) ;
		}

		static Event FromMSG(const MSG& msg) noexcept {
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
					return Event::CreateNormalEvent(nullptr, EventType::Quit) ;
				case WM_CLOSE : 
					return Event::CreateNormalEvent(msg.hwnd, EventType::Close) ;
			}
			return Event::CreateNormalEvent(msg.hwnd, EventType::None) ;
		}

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

		constexpr void* GetSliderAddress() const noexcept {
			return data_.slider_.slider_ptr_ ;
		}

		constexpr ButtonState GetButtonState() const noexcept {
			return data_.button_.state_ ;
		}

		constexpr void* GetButtonAddress() const noexcept {
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
	} ;

	class EventSystem {
	private :
		static inline std::queue<Event> g_events_ ;
		static inline bool event_was_initialized_ = false ;

	public :
		static void Initialize() noexcept {
			if (!event_was_initialized_) {
				event_was_initialized_ = true ;
				logger::info("EventSystem initialized!.") ;
				return ;
			}
			logger::info("EventSystem was initialized.") ;
		}

		static void PushEvent(const Event& e) noexcept {
			g_events_.push(e) ;
		}

		static bool PoolEvent(Event& e) noexcept {
			if (g_events_.empty()) { 
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
		}
	} ;

	std::string EventDescribe(const Event& e) noexcept {
		std::string msg = "\nEvent Type\t: " ;
		switch (e) {
			case EventType::None : 
				msg += "None\n" ;
				break ;

			case EventType::Quit : 
				msg += "Quit\n" ; 
				break ;

			case EventType::Resize : 
				msg += "Resize\nSize\t\t: {" + std::to_string(e.GetResizedSize().x) + ", " + std::to_string(e.GetResizedSize().y) + "}\n" ;
				break ;

			case EventType::Close : 
				msg += "Close\n" ;
				break ;
				

			case EventType::Key : 
				msg += "Key\nState\t\t: " ;
				if (e.GetKeyState() == KeyState::Down) {
					msg += "Down\n" ;
				} else if (e.GetKeyState() == KeyState::Up) {
					msg += "Up\n" ;
				} else {
					msg += "Unknown\n" ;
				}
				msg += "Value\t\t: " + std::to_string(e.GetKeyCode()) + '\n' ;
				break ;

			case EventType::Mouse :
				msg += "Mouse\nButton\t\t: " ;
				if (e.GetMouseButton() == MouseButton::None) {
					msg += "None\n" ;
				} else if (e.GetMouseButton() == MouseButton::Left) {
					msg += "Left\n" ;
				} else if (e.GetMouseButton() == MouseButton::Right) {
					msg += "Right\n" ;
				} else {
					msg += "Middle\n" ;
				}

				msg += "State\t\t: " ;
				if (e.GetMouseState() == MouseState::None) {
					msg += "None\n" ;
				} else if (e.GetMouseState() == MouseState::Up) {
					msg += "Up\n" ;
				} else if (e.GetMouseState() == MouseState::Down) {
					msg += "Down\n" ;
				} else {
					msg += "Wheel\n" ;
				}
				msg += "Position\t: {" + std::to_string(e.GetMousePosition().x) + ", " + std::to_string(e.GetMousePosition().y) + "}\n" ;
				break ;

			case EventType::Slider : 
				msg += "Slider\nState\t\t: " ;
				if (e.GetSliderState() == SliderState::None) {
					msg += "None\n" ;
				} else if (e.GetSliderState() == SliderState::Changed) {
					msg += "Changed\n" ;
				} else if (e.GetSliderState() == SliderState::Start) {
					msg += "Start\n" ;
				} else if (e.GetSliderState() == SliderState::End) {
					msg += "End\n" ;
				}
				
				msg += "Value\t\t: " + std::to_string(e.GetSliderValue()) + '\n' ;
				msg += "Address\t: " + std::to_string(reinterpret_cast<uintptr_t>(e.GetSliderAddress())) + '\n' ;
				break ;

			default :
				msg += "None" ;
				break ;
		}
		return msg ;
	}

	constexpr bool operator==(uint32_t a, KeyCode b) noexcept {
		return a == static_cast<uint32_t>(b) ;
	}

	constexpr bool operator!=(uint32_t a, KeyCode b) noexcept {
		return a != static_cast<uint32_t>(b) ;
	}

	static inline bool PollEvent(Event& e) {
		if (EventSystem::PoolEvent(e)) {
			return true ;
		}

		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				logger::info("PollEvent: WM_QUIT received via PeekMessage.") ;
				e = Event::CreateNormalEvent(nullptr, EventType::Quit) ;
				return true ;
			}

			Event ecvt = Event::FromMSG(msg) ;

			if (ecvt != EventType::None) {
				EventSystem::PushEvent(ecvt) ;
			}

			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
		}

		return EventSystem::PoolEvent(e) ;
	}

}