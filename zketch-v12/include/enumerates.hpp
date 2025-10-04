#pragma once

#include "env.hpp"
#include "win32init.hpp"
#include "gdiplusinit.hpp"

namespace zketch {

	// Enum WindowStyle modern + combos
	enum class WindowStyle : uint32_t {
		// Single flags
		HREDRAW          = CS_HREDRAW,         // Repaint window saat lebarnya berubah
		VREDRAW          = CS_VREDRAW,         // Repaint window saat tingginya berubah
		DOUBLECLICK      = CS_DBLCLKS,         // Aktifkan event double-click
		OWNDC            = CS_OWNDC,           // Window punya Device Context sendiri
		CLASSDC          = CS_CLASSDC,         // Semua window class share satu Device Context
		PARENTDC         = CS_PARENTDC,        // Gunakan DC dari parent window
		NOCLOSE          = CS_NOCLOSE,         // Window tidak punya tombol close
		SAVEBITS         = CS_SAVEBITS,        // Simpan area window yang tertutup popup (Windows klasik)
		BYTEALIGNCLIENT  = CS_BYTEALIGNCLIENT, // Align area client ke byte boundary
		BYTEALIGNWINDOW  = CS_BYTEALIGNWINDOW, // Align seluruh window ke byte boundary
		GLOBALCLASS      = CS_GLOBALCLASS,     // Window class global
		IME              = CS_IME,             // Aktifkan IME
		DROPSHADOW       = CS_DROPSHADOW,      // Tambahkan shadow/bayangan

		// Predefined combos
		DEFAULT          = HREDRAW | VREDRAW | DOUBLECLICK,
		TRACKBAR         = HREDRAW | NOCLOSE,
		DIALOG           = HREDRAW | VREDRAW | OWNDC | NOCLOSE,
		GRAPHICS         = HREDRAW | VREDRAW | OWNDC | CLASSDC,
		SHADOWED         = HREDRAW | VREDRAW | DROPSHADOW
	} ;

	// Bitwise OR operator
	inline WindowStyle operator|(WindowStyle a, WindowStyle b) {
		return static_cast<WindowStyle>(
			static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
		) ;
	}

	// Bitwise OR assignment
	inline WindowStyle& operator|=(WindowStyle& a, WindowStyle b) {
		a = a | b ;
		return a ;
	}

	template <typename From, typename To = int32_t, typename = std::enable_if_t<std::is_integral_v<To> && std::is_convertible_v<From, To>>>
	inline To FromFlag(From from) noexcept {
		return static_cast<To>(from) ;
		char c = '\'' ;
	}

	// windows compatibel
	enum class KeyCode : uint32_t {
		#if defined (_WIN32) || defined (_WIN64)
			// --- Alphanumeric ---
			Num0 = 0x30, Num1, Num2, Num3, Num4, 
			Num5, Num6, Num7, Num8, Num9,

			A = 0x41, B, C, D, E, F, G, H, I, J,
			K, L, M, N, O, P, Q, R, S, T,
			U, V, W, X, Y, Z,

			// --- Punctuation & Symbol ---
			Escape     = VK_ESCAPE,      // 27
			Tab        = VK_TAB,         // 9
			Enter      = VK_RETURN,      // 13
			Space      = VK_SPACE,       // 32
			Backspace  = VK_BACK,        // 8

			Minus        = VK_OEM_MINUS,      // 0xBD (189)
			Equal        = VK_OEM_PLUS,       // 0xBB (187)
			BracketLeft  = VK_OEM_4,          // 0xDB (219)
			BracketRight = VK_OEM_6,          // 0xDD (221)
			Backslash    = VK_OEM_5,          // 0xDC (220)
			Semicolon    = VK_OEM_1,          // 0xBA (186)
			Apostrophe   = VK_OEM_7,          // 0xDE (222)
			Comma        = VK_OEM_COMMA,      // 0xBC (188)
			Period       = VK_OEM_PERIOD,     // 0xBE (190)
			Slash        = VK_OEM_2,          // 0xBF (191)
			Grave        = VK_OEM_3,          // 0xC0 (192)

			// --- Function Keys ---
			F1 = VK_F1, F2, F3, F4, F5, F6, 
			F7, F8, F9, F10, F11, F12,

			// --- Navigation ---
			PageUp     = VK_PRIOR,   // 33
			PageDown   = VK_NEXT,    // 34
			End        = VK_END,     // 35
			Home       = VK_HOME,    // 36
			ArrowLeft  = VK_LEFT,    // 37
			ArrowUp    = VK_UP,      // 38
			ArrowRight = VK_RIGHT,   // 39
			ArrowDown  = VK_DOWN,    // 40
			Insert     = VK_INSERT,  // 45
			Delete     = VK_DELETE,  // 46

			// --- Modifiers ---
			CapsLock   = VK_CAPITAL, // 20
			Shift      = VK_SHIFT,   // generic
			Ctrl       = VK_CONTROL, // generic

			ShiftLeft  = VK_LSHIFT,  // 160
			ShiftRight = VK_RSHIFT,  // 161
			CtrlLeft   = VK_LCONTROL,// 162
			CtrlRight  = VK_RCONTROL,// 163
			AltLeft    = VK_LMENU,   // 164
			AltRight   = VK_RMENU,   // 165

			WinLeft    = VK_LWIN,    // 91
			WinRight   = VK_RWIN,    // 92

			// --- System / Legacy ---
			PrintScreen = VK_SNAPSHOT,  // 44
			ScrollLock  = VK_SCROLL,    // 145
			Pause       = VK_PAUSE,     // 19

			// --- Keypad ---
			NumPad0       = VK_NUMPAD0,    // 96
			NumPad1       = VK_NUMPAD1,
			NumPad2       = VK_NUMPAD2,
			NumPad3       = VK_NUMPAD3,
			NumPad4       = VK_NUMPAD4,
			NumPad5       = VK_NUMPAD5,
			NumPad6       = VK_NUMPAD6,
			NumPad7       = VK_NUMPAD7,
			NumPad8       = VK_NUMPAD8,
			NumPad9       = VK_NUMPAD9,

			NumPadDivide   = VK_DIVIDE,   // 111
			NumPadMultiply = VK_MULTIPLY, // 106
			NumPadSubtract = VK_SUBTRACT, // 109
			NumPadAdd      = VK_ADD,      // 107
			NumPadEnter    = VK_RETURN,   // same VK as Enter, need context
			NumPadDecimal  = VK_DECIMAL   // 110
		#elif __linux__
			// next improvements
		#elif __APPLE__
			// next improvements
		#endif
	} ;

	enum class Anchor : uint8_t {
		Left		= 1 << 0,
		LeftTop		= 1 << 1,
		Top			= 1 << 2,
		RightTop	= 1 << 3,
		Right		= 1 << 4,
		RightBottom	= 1 << 5,
		Bottom		= 1 << 6,
		LeftBottom	= 1 << 7,
		Center		= 0b11111111,
	} ;

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

	enum class FontStyle : uint32_t {
        Regular     = Gdiplus::FontStyleRegular,
        Bold        = Gdiplus::FontStyleBold,
        Italic      = Gdiplus::FontStyleItalic,
        BoldItalic  = Gdiplus::FontStyleBold | Gdiplus::FontStyleItalic,
        Underline   = Gdiplus::FontStyleUnderline,
        Strikeout   = Gdiplus::FontStyleStrikeout
    } ;
}