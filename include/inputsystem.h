#pragma once
#include "event.h"
#include "inputmanager.h"

namespace zketch {

    class InputSystem {
    private:
        InputManager input_ ;

    public:
        void update() noexcept {
            input_.update() ;
        }

        void handleEvent(const Event& e) noexcept {
            switch (e.type_) {
            case EventType::KeyDown:
                input_.setKeyDown(e.data_.key_.keyCode) ;
                break ;

            case EventType::KeyUp:
                input_.setKeyUp(e.data_.key_.keyCode) ;
                break ;

            case EventType::MouseDown:
                input_.setMouseDown(e.data_.mouse_.button) ;
                input_.setMousePos(e.data_.mouse_.pos) ;
                break ;

            case EventType::MouseUp:
                input_.setMouseUp(e.data_.mouse_.button) ;
                input_.setMousePos(e.data_.mouse_.pos) ;
                break ;

            case EventType::MouseMove:
                input_.setMousePos(e.data_.mouse_.pos) ;
                break ;

            case EventType::Resize:
                // optional: next improvement
                break ;

            default:
                break ;
            }
        }

        const InputManager& getInput() const noexcept { 
            return input_ ; 
        }
        InputManager& getInput() noexcept { 
            return input_ ; 
        }
    } ;
}
