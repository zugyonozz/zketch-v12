#pragma once

#include "event.hpp"

namespace zketch {

class InputSystem {
private:
    std::bitset<256> KeyDown_ ;
	std::bitset<256> KeyReleased_ ; 
	std::bitset<256> KeyPressed_ ;
    std::bitset<3> MouseDown_ ; 
	std::bitset<3> MouseReleased_;
	std::bitset<3> MousePressed_ ;
    Point mouse_pos_ ;
    int64_t mouse_delta_ = 0 ;

public:
    constexpr InputSystem() noexcept = default ;

    void Update() noexcept {
        KeyPressed_.reset() ;
        KeyReleased_.reset() ;

        MousePressed_.reset() ;
        MouseReleased_.reset() ;

        mouse_delta_ = 0 ;
    }

    
    void SetKeyDown(uint32_t key) noexcept {
        if (key < KeyDown_.size()) {
            if (!KeyDown_[key]) {
                KeyPressed_[key] = true ; 
            }
            KeyDown_[key] = true ; 
        }
    }

    void SetKeyUp(uint32_t key) noexcept {
        if (key < KeyDown_.size()) {
            if (KeyDown_[key]) {
                KeyReleased_[key] = true ; 
            }
            KeyDown_[key] = false ; 
        }
    }

    
    void SetKeyDown(KeyCode key) noexcept { 
		SetKeyDown(static_cast<uint32_t>(key)) ; 
	}

    void SetKeyUp(KeyCode key) noexcept { 
		SetKeyUp(static_cast<uint32_t>(key)) ; 
	}

    void SetMouseDown(uint32_t button) noexcept {
        if (button < MouseDown_.size()) {
            if (!MouseDown_[button]) MousePressed_[button] = true ;
            MouseDown_[button] = true ;
        }
    }

    void SetMouseUp(uint32_t button) noexcept {
        if (button < MouseDown_.size()) {
            if (MouseDown_[button]) MouseReleased_[button] = true ;
            MouseDown_[button] = false ;
        }
    }

    void SetMousePos(const Point& pos) noexcept {
        mouse_pos_ = pos ;
    }

    void SetMouseDelta(int16_t delta) noexcept { 
		mouse_delta_ = delta ; 
	}

    bool IsKeyDown(uint32_t key) const noexcept { 
		return key < KeyDown_.size() && KeyDown_[key] ; 
	}

    bool IsKeyPressed(uint32_t key) const noexcept { 
		return key < KeyPressed_.size() && KeyPressed_[key] ; 
	}

    bool IsKeyReleased(uint32_t key) const noexcept { 
		return key < KeyReleased_.size() && KeyReleased_[key] ; 
	}

    bool IsKeyDown(KeyCode key) const noexcept { 
		return IsKeyDown(static_cast<uint32_t>(key)) ; 
	}

    bool IsKeyPressed(KeyCode key) const noexcept { 
		return IsKeyPressed(static_cast<uint32_t>(key)) ; 
	}

    bool IsKeyReleased(KeyCode key) const noexcept { 
		return IsKeyReleased(static_cast<uint32_t>(key)) ; 
	}

    bool IsMouseDown(uint32_t button) const noexcept { 
		return button < MouseDown_.size() && MouseDown_[button] ; 
	}

    bool IsMousePressed(uint32_t button) const noexcept { 
		return button < MousePressed_.size() && MousePressed_[button] ; 
	}

    bool IsMouseReleased(uint32_t button) const noexcept { 
		return button < MouseReleased_.size() && MouseReleased_[button] ; 
	}

    Point GetMousePos() const noexcept { 
		return mouse_pos_ ; 
	}

    int16_t GetMouseDelta() const noexcept { 
		return mouse_delta_ ;
	}
 
    bool IsShiftDown() const noexcept { 
		return IsKeyDown(static_cast<uint32_t>(VK_SHIFT)) ; 
	}

    bool IsCtrlDown() const noexcept { 
		return IsKeyDown(static_cast<uint32_t>(VK_CONTROL)) ; 
	}

    bool IsAltDown() const noexcept { 
		return IsKeyDown(static_cast<uint32_t>(VK_MENU)) ; 
	}

} ;

} 
