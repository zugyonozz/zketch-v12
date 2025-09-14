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
    Point MousePos_, MouseDelta_ ;
    float MouseWheel_ = 0.0f ;

public:
    constexpr InputSystem() noexcept = default ;

    void Update() noexcept {
        KeyPressed_.reset() ;
        KeyReleased_.reset() ;

        MousePressed_.reset() ;
        MouseReleased_.reset() ;

        MouseDelta_ = {0, 0} ;
        MouseWheel_ = 0.0f ;
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

    void setMouseDown(uint32_t button) noexcept {
        if (button < MouseDown_.size()) {
            if (!MouseDown_[button]) MousePressed_[button] = true ;
            MouseDown_[button] = true ;
        }
    }

    void setMouseUp(uint32_t button) noexcept {
        if (button < MouseDown_.size()) {
            if (MouseDown_[button]) MouseReleased_[button] = true ;
            MouseDown_[button] = false ;
        }
    }

    void setMousePos(const Point& pos) noexcept {
        MouseDelta_ = pos - MousePos_ ;
        MousePos_ = pos ;
    }

    void setMouseWheel(float delta) noexcept { 
		MouseWheel_ = delta ; 
	}

    bool isKeyDown(uint32_t key) const noexcept { 
		return key < KeyDown_.size() && KeyDown_[key] ; 
	}

    bool isKeyPressed(uint32_t key) const noexcept { 
		return key < KeyPressed_.size() && KeyPressed_[key] ; 
	}

    bool isKeyReleased(uint32_t key) const noexcept { 
		return key < KeyReleased_.size() && KeyReleased_[key] ; 
	}

    bool isKeyDown(KeyCode key) const noexcept { 
		return isKeyDown(static_cast<uint32_t>(key)) ; 
	}

    bool isKeyPressed(KeyCode key) const noexcept { 
		return isKeyPressed(static_cast<uint32_t>(key)) ; 
	}

    bool isKeyReleased(KeyCode key) const noexcept { 
		return isKeyReleased(static_cast<uint32_t>(key)) ; 
	}

    bool isMouseDown(uint32_t button) const noexcept { 
		return button < MouseDown_.size() && MouseDown_[button] ; 
	}

    bool isMousePressed(uint32_t button) const noexcept { 
		return button < MousePressed_.size() && MousePressed_[button] ; 
	}

    bool isMouseReleased(uint32_t button) const noexcept { 
		return button < MouseReleased_.size() && MouseReleased_[button] ; 
	}

    Point getMousePos() const noexcept { 
		return MousePos_ ; 
	}

    Point getMouseDelta() const noexcept { 
		return MouseDelta_ ; 
	}

    float getMouseWheel() const noexcept { 
		return MouseWheel_ ; 
	}
 
    bool isShiftDown() const noexcept { 
		return isKeyDown(static_cast<uint32_t>(VK_SHIFT)) ; 
	}

    bool isCtrlDown() const noexcept { 
		return isKeyDown(static_cast<uint32_t>(VK_CONTROL)) ; 
	}

    bool isAltDown() const noexcept { 
		return isKeyDown(static_cast<uint32_t>(VK_MENU)) ; 
	}

} ;

} 
