#pragma once

#include "unit.h"

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
		Close
	} ;

	enum class MouseButton : uint8_t { 
		Unknown,
		Left, 
		Right, 
		Middle, 
	} ;

	class Event {
	private :
		EventType type_ = EventType::None ;
		HWND hwnd_ = nullptr ;

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
		} data_ ;

		// ========================================= //
		//                  Construtor               //
		// ========================================= // 

		constexpr Event(HWND src_, EventType type_, uint32_t key_code_)  {
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

		constexpr Event(HWND src_, EventType type_) {
			this->type_ = type_ ;
			if ((isMouseEvent() || iskeyEvent() || isResizeEvent()))
				throw error_handler::invalid_event_type() ;
			data_.empty_ = {} ;
			hwnd_ = src_ ;
		}

	public :

		// ========================================= //
		//              Checking Method              //
		// ========================================= // 

		constexpr bool iskeyEvent() const noexcept {
			return (type_ == EventType::KeyDown || type_ == EventType::KeyUp) ;
		}

		constexpr bool isMouseEvent() const noexcept {
			return (type_ == EventType::MouseDown || type_ == EventType::MouseUp || type_ == EventType::MouseMove) ;
		}

		constexpr bool isResizeEvent() const noexcept {
			return (type_ == EventType::Resize) ;
		}

		// ========================================= //
		//              Creator Factory              //
		// ========================================= // 

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

		static Event getEvent(const MSG& msg) noexcept {
			switch (msg.message) {
				case WM_KEYDOWN : return Event::createKeyEvent(msg.hwnd, EventType::KeyDown, msg.wParam) ; 
				case WM_KEYUP : return Event::createKeyEvent(msg.hwnd, EventType::KeyUp, msg.wParam) ; 
				case WM_MOUSEMOVE : return Event::createMouseEvent(msg.hwnd, EventType::MouseMove, MouseButton::Unknown, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONDOWN : return Event::createMouseEvent(msg.hwnd, EventType::MouseDown, MouseButton::Left, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONDOWN : return Event::createMouseEvent(msg.hwnd, EventType::MouseDown, MouseButton::Right, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_LBUTTONUP : return Event::createMouseEvent(msg.hwnd, EventType::MouseUp, MouseButton::Left, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_RBUTTONUP : return Event::createMouseEvent(msg.hwnd, EventType::MouseUp, MouseButton::Right, {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)}) ; 
				case WM_SIZE : return Event::createResizeEvent(msg.hwnd, {LOWORD(msg.lParam), HIWORD(msg.lParam)}) ;
				case WM_QUIT : return Event::createEvent(msg.hwnd, EventType::Quit) ;
				case WM_CLOSE : return Event::createEvent(msg.hwnd, EventType::Close) ;
			}

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

		constexpr operator EventType() const noexcept {
			return type_ ;
		}

		const HWND getHandle() const noexcept {
			return hwnd_ ;
		}
	} ;
}