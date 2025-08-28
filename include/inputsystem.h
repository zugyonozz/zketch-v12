#pragma once
#include "event.h"
#include "inputmanager.h"

namespace zketch {

    class InputSystem {
    private:
        InputManager input_ ;

    public:
        void update() noexcept {
            input_.PrepareForNewFrame() ;
        }

        void handleEvent(const Event& e) noexcept {
            switch (e.type_) {
            case EventType::KeyDown:
                if (auto keyCode = e.getKeyCode()) {
                    input_.SetKeyDown(*keyCode);
                }
                break ;

            case EventType::KeyUp:
                if (auto keyCode = e.getKeyCode()) {
                    input_.SetKeyUp(*keyCode);
                }
                break ;

            case EventType::MouseDown:
                if (auto pos = e.getMousePos()) {
                    input_.SetMousePosition(*pos);
                }
                // Extract button from mouse event
                if (e.isMouseEvent()) {
                    // We need to get the button from the event data
                    // This assumes the button is stored correctly in the event
                    if (e.isMouse(MouseButton::Left)) {
                        input_.SetMouseDown(static_cast<int>(MouseButton::Left));
                    } else if (e.isMouse(MouseButton::Right)) {
                        input_.SetMouseDown(static_cast<int>(MouseButton::Right));
                    } else if (e.isMouse(MouseButton::Middle)) {
                        input_.SetMouseDown(static_cast<int>(MouseButton::Middle));
                    }
                }
                break ;

            case EventType::MouseUp:
                if (auto pos = e.getMousePos()) {
                    input_.SetMousePosition(*pos);
                }
                // Extract button from mouse event
                if (e.isMouseEvent()) {
                    if (e.isMouse(MouseButton::Left)) {
                        input_.SetMouseUp(static_cast<int>(MouseButton::Left));
                    } else if (e.isMouse(MouseButton::Right)) {
                        input_.SetMouseUp(static_cast<int>(MouseButton::Right));
                    } else if (e.isMouse(MouseButton::Middle)) {
                        input_.SetMouseUp(static_cast<int>(MouseButton::Middle));
                    }
                }
                break ;

            case EventType::MouseMove:
                if (auto pos = e.getMousePos()) {
                    input_.SetMousePosition(*pos);
                }
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