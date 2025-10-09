#pragma once
#include "widget copy.hpp"

namespace zketch {
	class Button : public Widget<Button> {
		friend class Widget<Button> ;

    private:
        bool is_hovered_ = false ;
        bool is_pressed_ = false ;
        std::wstring label_ ;
        Font font_ ;
        std::function<void(Canvas*, const Button&)> drawer_ ;
        std::function<void()> on_click_ ;

		void UpdateImpl() noexcept {
            if (!drawer_) {
				return ;
			}

            if (!ValidateCanvas("Button::UpdateImpl()")) {
				return ;
			}

            drawer_(canvas_.get(), *this) ;
        }

    public:
        Button(const RectF& bound, const std::wstring& label = L"", const Font& font = Font()) noexcept : label_(label), font_(font) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
            canvas_->SetClearColor(rgba(0, 0, 0, 0)) ;

            SetDrawer([](Canvas* canvas, const Button& button) {
                Drawer drawer ;
                if (!drawer.Begin(*canvas)) return ;

                Color button_color ;
                Color border_color ;
                
                if (button.IsPressed()) {
                    button_color = rgba(70, 130, 180, 255) ;
                    border_color = rgba(50, 100, 150, 255) ;
                } else if (button.IsHovered()) {
                    button_color = rgba(100, 149, 237, 255) ;
                    border_color = rgba(70, 119, 207, 255) ;
                } else {
                    button_color = rgba(135, 206, 250, 255) ;
                    border_color = rgba(100, 171, 220, 255) ;
                }
                
                RectF rect = button.GetRelativeBound() ;
                drawer.FillRectRounded(rect, button_color, 5.0f) ;
                drawer.DrawRectRounded(rect, border_color, 5.0f, 2.0f) ;
                
                if (!button.GetLabel().empty()) {
                    Color text_color = rgba(255, 255, 255, 255) ;
                    Point text_pos = {
                        static_cast<int32_t>(rect.w / 2 - 30),
                        static_cast<int32_t>(rect.h / 2 - 10)
                    } ;
                    drawer.DrawString(button.GetLabel(), text_pos, text_color, button.GetFont()) ;
                }

                drawer.End() ;
            }) ;
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                MarkDirty() ;
            }
            return state ;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (bound_.Contain(mouse_pos)) {
                is_pressed_ = true ;
                MarkDirty() ;
                return true ;
            }
            return false ;
        }

        bool OnRelease(const PointF& mouse_pos) noexcept {
            bool was_pressed = is_pressed_ ;
            is_pressed_ = false ;
            if (was_pressed) {
                MarkDirty() ;
                if (bound_.Contain(mouse_pos) && on_click_) {
                    on_click_() ;
                }
                return bound_.Contain(mouse_pos) ;
            }
            return false ;
        }

        void PresentImpl(Window& window) noexcept {
            if (!ValidateCanvas("Button::PresentImpl()")) {
				return ;
			}
			
			window.Present(*canvas_, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
        }

        void SetDrawer(std::function<void(Canvas*, const Button&)> drawer) noexcept {
            drawer_ = std::move(drawer) ;
            MarkDirty() ;
        }
        
        void SetOnClick(std::function<void()> callback) noexcept {
            on_click_ = std::move(callback) ;
        }
        
        void SetLabel(const std::wstring& label) noexcept {
            if (label_ != label) {
                label_ = label ;
                MarkDirty() ;
            }
        }

        RectF GetRelativeBound() const noexcept {
            return {0, 0, bound_.w, bound_.h} ;
        }

        const std::wstring& GetLabel() const noexcept { 
			return label_ ; 
		}

        const Font& GetFont() const noexcept { 
			return font_ ; 
		}

        bool IsHovered() const noexcept { 
			return is_hovered_ ; 
		}

        bool IsPressed() const noexcept { 
			return is_pressed_ ; 
		}
    } ;
}