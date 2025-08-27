#pragma once

#include <cstdint>

#include "win32init.h"

namespace zketch {

    enum class KeyCode : uint16_t {
		Unknown = 0,
        // --- Alphabet ---
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // --- Numeric ---
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        // --- Function ---
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        // --- Modifier ---
        LeftShift = 256, RightShift,
        LeftControl, RightControl,
        LeftAlt, RightAlt,

        // --- Special ---
        Space, Enter, Esc, Tab, Backspace,
        
        // --- Arrow ---
        Up, Down, Left, Right,

		// size
        Count 
    } ;

	inline KeyCode TranslateVirtualKey(WPARAM vk_code) {
        switch (vk_code) {
            case VK_SHIFT :   return KeyCode::LeftShift ; // Bisa dibedakan lagi nanti
            case VK_CONTROL : return KeyCode::LeftControl ;
            case VK_MENU :    return KeyCode::LeftAlt ;
            case VK_SPACE :   return KeyCode::Space ;
            case VK_RETURN :  return KeyCode::Enter ;
            case VK_ESCAPE :  return KeyCode::Esc ;
            case VK_TAB :     return KeyCode::Tab ;
            case VK_BACK :    return KeyCode::Backspace ;
            case VK_UP :      return KeyCode::Up ;
            case VK_DOWN :    return KeyCode::Down ;
            case VK_LEFT :    return KeyCode::Left ;
            case VK_RIGHT :   return KeyCode::Right ;
            default :
                if (vk_code >= 'A' && vk_code <= 'Z') return static_cast<KeyCode>(vk_code) ;
                if (vk_code >= '0' && vk_code <= '9') return static_cast<KeyCode>(vk_code) ;
                return KeyCode::Unknown ;
        }
    }

}
