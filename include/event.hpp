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
		KeyDown, 
		KeyUp, 
		MouseDown, 
		MouseUp, 
		MouseMove, 
		Resize, 
		Close,
		TrackBar,
	} ;

	enum class MouseButton : uint8_t { 
		Unknown,
		Left, 
		Right, 
		Middle, 
	} ;

	enum class TrackBarType : uint8_t {
		VScroll,
		HScroll,
	} ;

	class Event {
	private :
		EventType type_ = EventType::None ;
		HWND hwnd_ = nullptr ;
		uint64_t timestamp_ = 0 ;

		union data_ {
			struct empty__ {} empty_ ;
			uint32_t key_code_ ;
			struct resize__ {
				uint32_t width_, height_ ;
			} resize_ ;
			struct mouse__ {
				uint8_t button_ ;
				uint32_t x_, y_ ;
			} mouse_ ;
			struct trackbar__ {
				uint8_t trackbarType_ ;
				size_t value_ ;
			} trackbar_ ;
		} data_ ;

		// -------------- Construtor  --------------

		constexpr Event(HWND src_, EventType type_, uint32_t key_code_) {
			this->type_ = type_ ;
			if (!iskeyEvent()) 
				throw error_handler::invalid_event_type() ;
			data_.key_code_ = key_code_ ;
			hwnd_ = src_ ;
		}

		constexpr Event(HWND src_, const Point_<uint32_t>& size_) noexcept {
			type_ = EventType::Resize ;
			data_.resize_ = {size_.x, size_.y} ;
			hwnd_ = src_ ;
		}

		constexpr Event(HWND src_, EventType type_, MouseButton button_, const Point_<uint32_t>& pos_) {
			this->type_ = type_ ;
			if (!isMouseEvent())
				throw error_handler::invalid_event_type() ;
			if (type_ == EventType::MouseMove)
				data_.mouse_ = {0, pos_.x, pos_.y} ;
			else
				data_.mouse_ = {static_cast<uint8_t>(button_), pos_.x, pos_.y} ;
			hwnd_ = src_ ;
		}

		constexpr Event(HWND src_, TrackBarType scrolltype_, size_t value_) noexcept {
			this->type_ = EventType::TrackBar ;
			data_.trackbar_ = {static_cast<uint8_t>(scrolltype_), value_} ;
			hwnd_ = src_ ;
		}

		constexpr Event(HWND src_, EventType type_) {
			this->type_ = type_ ;
			if ((isMouseEvent() || iskeyEvent() || isResizeEvent()))
				throw error_handler::invalid_event_type() ;
			data_.empty_ = {} ;
			hwnd_ = src_ ;
		}

	public :

		constexpr Event() noexcept : type_(EventType::None), hwnd_(nullptr) {
			data_.empty_ = {} ;
		}

		// -------------- Checking Method  --------------

		constexpr bool iskeyEvent() const noexcept {
			return (type_ == EventType::KeyDown || type_ == EventType::KeyUp) ;
		}

		constexpr bool isMouseEvent() const noexcept {
			return (type_ == EventType::MouseDown || type_ == EventType::MouseUp || type_ == EventType::MouseMove) ;
		}

		constexpr bool isResizeEvent() const noexcept {
			return (type_ == EventType::Resize) ;
		}

		// -------------- Creator Factory  -------------- 

		static Event createEvent() noexcept {
			return Event(nullptr, EventType::None) ;
		}

		static Event createEvent(HWND src_, EventType type_) noexcept {
			return Event(src_, type_) ;
		}

		static Event createKeyEvent(HWND src_, EventType type_, uint32_t key_code_) noexcept {
			return Event(src_, type_, key_code_) ;
		}

		static Event createMouseEvent(HWND src_, EventType type_, MouseButton button_, const Point& pos_) noexcept {
			return Event(src_, type_, button_, pos_) ;
		}

		static Event createResizeEvent(HWND src_, const Point& size_) noexcept {
			return Event(src_, size_) ;
		}

		static Event createScrollEvent(HWND src_, TrackBarType scrolltype_, size_t value) noexcept {
			return Event(src_, scrolltype_, value) ;
		}

		static Event FromMSG(const MSG& msg) noexcept {
			// logger::info("FromMSG: Converting message ", msg.message, " from HWND ", msg.hwnd) ;
			
			switch (msg.message) {
				case WM_KEYDOWN : 
					// logger::info("-> Converting to KeyDown event") ;
					return Event::createKeyEvent(msg.hwnd, EventType::KeyDown, msg.wParam) ; 
				case WM_KEYUP : 
					// logger::info("-> Converting to KeyUp event") ;
					return Event::createKeyEvent(msg.hwnd, EventType::KeyUp, msg.wParam) ; 
				case WM_MOUSEMOVE : 
					// logger::info("-> Converting to MouseMove event") ;
					return Event::createMouseEvent(msg.hwnd, EventType::MouseMove, MouseButton::Unknown, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONDOWN : 
					// logger::info("-> Converting to MouseDown (Left) event") ;
					return Event::createMouseEvent(msg.hwnd, EventType::MouseDown, MouseButton::Left, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONDOWN : 
					// logger::info("-> Converting to MouseDown (Right) event") ;
					return Event::createMouseEvent(msg.hwnd, EventType::MouseDown, MouseButton::Right, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONUP : 
					// logger::info("-> Converting to MouseUp (Left) event") ;
					return Event::createMouseEvent(msg.hwnd, EventType::MouseUp, MouseButton::Left, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONUP : 
					// logger::info("-> Converting to MouseUp (Right) event") ;
					return Event::createMouseEvent(msg.hwnd, EventType::MouseUp, MouseButton::Right, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_QUIT : 
					// logger::info("-> Converting to Quit event") ;
					return Event::createEvent(nullptr, EventType::Quit) ;
				case WM_CLOSE : 
					// logger::info("-> Converting to Close event") ;
					return Event::createEvent(msg.hwnd, EventType::Close) ;
			}

			// logger::info("-> Converting to None event (unhandled message)") ;
			return Event::createEvent(msg.hwnd, EventType::None) ;
		}

		constexpr uint32_t keyCode() const noexcept { 
			return data_.key_code_ ; 
		}

		constexpr Point_<uint32_t> resizeSize() const noexcept { 
			return {data_.resize_.width_, data_.resize_.height_} ; 
		}

		constexpr Point_<uint32_t> mousePos() const noexcept { 
			return {data_.mouse_.x_, data_.mouse_.y_} ; 
		}

		constexpr MouseButton mouseButton() const noexcept { 
			return static_cast<MouseButton>(data_.mouse_.button_) ; 
		}

		constexpr TrackBarType scrollBarType() const noexcept {
			return static_cast<TrackBarType>(data_.trackbar_.trackbarType_) ;
		}

		constexpr size_t scrollValue() const noexcept {
			return data_.trackbar_.value_ ;
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

		HWND getHandle() const noexcept {
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

		static void Clear() noexcept {
			while (!g_events_.empty()) {
				g_events_.pop() ;
			}
		}
	} ;

	std::string getEventString(const Event& e) noexcept {
		switch (e.GetEventType()) {
			case EventType::None : return "None" ;
			case EventType::Quit : return "Quit" ;
			case EventType::KeyDown : return "KeyDown" ;
			case EventType::KeyUp : return "KeyUp" ;
			case EventType::MouseDown : return "MouseDown" ;
			case EventType::MouseUp : return "MouseUp" ;
			case EventType::MouseMove : return "MouseMove" ;
			case EventType::Resize : return "Resize" ;
			case EventType::Close : return "Close" ;
			case EventType::TrackBar : return "ScrollBar" ;
		}
		return "Unknown" ;
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
				e = Event::createEvent(nullptr, EventType::Quit) ;
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