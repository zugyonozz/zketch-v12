#pragma once

#include <cstdint>

namespace zketch {

    enum class KeyCode : uint8_t {
        // --- Alphabet ---
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // --- Numeric ---
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        // --- Function ---
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        // --- Modifier ---
        LeftShift, RightShift,
        LeftControl, RightControl,
        LeftAlt, RightAlt,

        // --- Special ---
        Space, Enter, Esc, Tab, Backspace,
        
        // --- Arrow ---
        Up, Down, Left, Right,

		// size
        Count 
    } ;

}
