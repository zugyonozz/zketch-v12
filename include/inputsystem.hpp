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
    int64_t mouse_wheel_ = 0 ;

public:
    constexpr InputSystem() noexcept = default ;

    void Update() noexcept {
        KeyPressed_.reset() ;
        KeyReleased_.reset() ;

        MousePressed_.reset() ;
        MouseReleased_.reset() ;

        mouse_wheel_ = 0 ;
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

	void SetMouseDown(MouseButton button) noexcept {
        SetMouseDown(static_cast<uint8_t>(button)) ;
    }

    void SetMouseUp(uint32_t button) noexcept {
        if (button < MouseDown_.size()) {
            if (MouseDown_[button]) MouseReleased_[button] = true ;
            MouseDown_[button] = false ;
        }
    }

	void SetMouseUp(MouseButton button) noexcept {
        SetMouseUp(static_cast<uint32_t>(button)) ;
    }

    void SetMousePos(const Point& pos) noexcept {
        mouse_pos_ = pos ;
    }

    void SetMouseWheel(int16_t value) noexcept { 
		mouse_wheel_ = value ; 
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

	bool IsMousePressed(MouseButton button) const noexcept { 
		return IsMousePressed(static_cast<uint8_t>(button)) ;
	}

    bool IsMouseReleased(uint32_t button) const noexcept { 
		return button < MouseReleased_.size() && MouseReleased_[button] ; 
	}

	bool IsMouseReleased(MouseButton button) const noexcept { 
		return IsMouseReleased(static_cast<uint8_t>(button)) ;
	}

    Point GetMousePos() const noexcept { 
		return mouse_pos_ ; 
	}

    int16_t GetMouseWheel() const noexcept { 
		return mouse_wheel_ ;
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
