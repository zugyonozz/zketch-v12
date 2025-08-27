#pragma once

#ifndef USE_ZKETCH_ALIAS
	#define USE_ZKETCH_ALIAS
#endif

#include <optional>
#include <variant>
#include <queue>
#include <unordered_map>

#include "unit.h"
#include "keycode.h"

namespace zketch {

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
		Left, 
		Right, 
		Middle, 
		Unknown 
	} ;

	namespace impl {

		struct KeyEventData {
			KeyCode sym ;
		} ;

		struct MouseEventData {
			Point pos ;
			uint8_t button ;
		} ;

		struct ResizeEventData {
			Point size ;
		} ;

	}

	struct EventTranslator ;

	class Event {
	private :
		using EventData = std::variant<std::monostate, impl::KeyEventData, impl::MouseEventData, impl::ResizeEventData> ;
		EventData data_ ;

		// private ctor
        Event(EventType type, HWND src) : type_(type), hwnd_(src) {}
        Event(EventType type, HWND src, EventData data) : type_(type), hwnd_(src), data_(std::move(data)) {}

	public :
		const EventType type_ = EventType::None ;
		const HWND hwnd_ = nullptr ;

		static Event CreateSimpleEvent(EventType type, HWND src) noexcept {
            return Event(type, src) ;
        }

        static Event CreateKeyEvent(EventType type, HWND src, KeyCode key) noexcept {
            return Event(type, src, impl::KeyEventData{key}) ;
        }

        static Event CreateMouseButtonEvent(EventType type, HWND src, uint8_t button, const Point& pos) noexcept {
            return Event(type, src, impl::MouseEventData{pos, button}) ;
        }

		static Event CreateResizeEvent(HWND src, const Point& size) noexcept {
			return Event(EventType::Resize, src, impl::ResizeEventData{size}) ;
		}

		// getter methods
		
		std::optional<KeyCode> getKeyCode() const noexcept {
			if (isKeyEvent())
				return std::get<impl::KeyEventData>(data_).sym ;
			return std::nullopt ;
		}

		std::optional<Point> getMousePos() const noexcept {
			if (isMouseEvent()) 
				return std::get<impl::MouseEventData>(data_).pos ;
			return std::nullopt ;
		}

		std::optional<Point> getResizeSize() const noexcept {
			if (isResizeEvent())
				return std::get<impl::ResizeEventData>(data_).size ;
			return std::nullopt ;
		}

		std::optional<impl::KeyEventData> AsKeyEvent() const noexcept {
			if (isKeyEvent())
				return std::get<impl::KeyEventData>(data_) ;
			return std::nullopt ;
		}

		// checker methods

		bool isKeyEvent() const noexcept {
			return (type_ == EventType::KeyDown || type_ == EventType::KeyUp) ;
		}

		bool isMouseEvent() const noexcept {
			return (type_ == EventType::MouseDown || type_ == EventType::MouseUp || type_ == EventType::MouseMove) ;
		}

		bool isResizeEvent() const noexcept {
			return type_ == EventType::Resize ;
		}

		bool isKey(KeyCode keyCode) const noexcept {
			return isKeyEvent() && std::get<impl::KeyEventData>(data_).sym == keyCode ;
		}

		bool isMouse(MouseButton button) const noexcept {
			return isMouseEvent() && std::get<impl::MouseEventData>(data_).button == static_cast<uint8_t>(button) ;
		}

		bool isWindow(HWND src) const noexcept {
			return src == hwnd_ ;
		}
	} ;

	struct EventTranslator {
		const Event operator()(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) const noexcept {
			switch (msg) {
				case WM_QUIT : return Event::CreateSimpleEvent(EventType::Quit, hwnd) ;
				case WM_CLOSE : return Event::CreateSimpleEvent(EventType::Close, hwnd) ;
				case WM_DESTROY : return Event::CreateSimpleEvent(EventType::Quit, hwnd) ;
				case WM_KEYDOWN : return Event::CreateKeyEvent(EventType::KeyDown, hwnd, TranslateVirtualKey(wp)) ;
				case WM_KEYUP : return Event::CreateKeyEvent(EventType::KeyUp, hwnd, TranslateVirtualKey(wp)) ;
				case WM_MOUSEMOVE : return Event::CreateMouseButtonEvent(EventType::MouseMove, hwnd, static_cast<uint8_t>(MouseButton::Unknown), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}) ;
				case WM_LBUTTONDOWN : return Event::CreateMouseButtonEvent(EventType::MouseDown, hwnd, static_cast<uint8_t>(MouseButton::Left), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}) ;
				case WM_RBUTTONDOWN : return Event::CreateMouseButtonEvent(EventType::MouseDown, hwnd, static_cast<uint8_t>(MouseButton::Right), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}) ;
				case WM_LBUTTONUP : return Event::CreateMouseButtonEvent(EventType::MouseUp, hwnd, static_cast<uint8_t>(MouseButton::Left), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}) ;
				case WM_RBUTTONUP : return Event::CreateMouseButtonEvent(EventType::MouseUp, hwnd, static_cast<uint8_t>(MouseButton::Right), {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}) ;
				case WM_SIZE : return Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)}) ;
				default : return Event::CreateSimpleEvent(EventType::None, hwnd) ;
			}
		} 
	} ;

}

#ifdef USE_ZKETCH_ALIAS
	using eventSequences = std::queue<zketch::Event> ;
	
	template <typename Key, typename Type>
	using fastmap = std::unordered_map<Key, Type> ;
#endif

#ifdef USE_ZKETCH_ALIAS
	#undef USE_ZKETCH_ALIAS
#endif