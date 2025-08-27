#pragma once

#include <bitset>
#include "event.h"

#include "KeyCode.h"

namespace zketch {

    class InputManager {
    private:
        static constexpr size_t KEY_COUNT = static_cast<size_t>(KeyCode::Count) ;

        static constexpr size_t MOUSE_BUTTON_COUNT = 3 ;

        std::bitset<KEY_COUNT> last_keys_held_ ;

        std::bitset<MOUSE_BUTTON_COUNT> last_mouse_buttons_held_ ;

        std::bitset<KEY_COUNT> keys_held_ ;

        std::bitset<MOUSE_BUTTON_COUNT> mouse_buttons_held_ ;

        Point mouse_pos_ ;

        float mouse_wheel_delta_ = 0.0f ;

    public:
        // Panggil ini SETIAP AWAL FRAME, SEBELUM memproses event.
        void PrepareForNewFrame() noexcept {
            last_keys_held_ = keys_held_ ;
            last_mouse_buttons_held_ = mouse_buttons_held_ ;
            mouse_wheel_delta_ = 0.0f;
        }

        void SetKeyDown(KeyCode key) noexcept {
            keys_held_[static_cast<size_t>(key)] = true ;
        }

        void SetKeyUp(KeyCode key) noexcept {
            keys_held_[static_cast<size_t>(key)] = false ;
        }

        void SetMouseDown(int button) noexcept {
            if (button >= 0 && button < MOUSE_BUTTON_COUNT) 
                mouse_buttons_held_[button] = true ;
        }

        void SetMouseUp(int button) noexcept {
            if (button >= 0 && button < MOUSE_BUTTON_COUNT) 
                mouse_buttons_held_[button] = false ;
        }

        void SetMousePosition(const Point& pos) noexcept {
            mouse_pos_ = pos ;
        }

        void AddMouseWheelDelta(float delta) noexcept {
            mouse_wheel_delta_ += delta ;
        }

        bool WasKeyPressed(KeyCode key) const noexcept {
            size_t index = static_cast<size_t>(key) ;
            return keys_held_[index] && !last_keys_held_[index] ;
        }

        bool IsKeyHeld(KeyCode key) const noexcept {
            return keys_held_[static_cast<size_t>(key)] ;
        }

        bool WasKeyReleased(KeyCode key) const noexcept {
            size_t index = static_cast<size_t>(key) ;
            return !keys_held_[index] && last_keys_held_[index] ;
        }

        bool WasMouseButtonPressed(MouseButton button) const noexcept {
            size_t index = static_cast<size_t>(button);
            return mouse_buttons_held_[index] && !last_mouse_buttons_held_[index] ;
        }

        bool IsMouseButtonHeld(MouseButton button) const noexcept {
            return mouse_buttons_held_[static_cast<size_t>(button)] ;
        }

        bool WasMouseButtonReleased(MouseButton button) const noexcept {
            size_t index = static_cast<size_t>(button);
            return !mouse_buttons_held_[index] && last_mouse_buttons_held_[index] ;
        }

        Point GetMousePosition() const noexcept {
            return mouse_pos_ ;
        }

        float GetMouseWheelDelta() const noexcept {
            return mouse_wheel_delta_ ;
        }

        bool IsShiftHeld() const noexcept {
            return IsKeyHeld(KeyCode::LeftShift) || IsKeyHeld(KeyCode::RightShift) ;
        }

        bool IsCtrlHeld() const noexcept {
            return IsKeyHeld(KeyCode::LeftControl) || IsKeyHeld(KeyCode::RightControl) ;
        }

        bool IsAltHeld() const noexcept {
            return IsKeyHeld(KeyCode::LeftAlt) || IsKeyHeld(KeyCode::RightAlt) ;
        }
    };

}