#pragma once
#include "widget.hpp"

namespace zketch {
	class Button : public Widget<Button> {
		friend class Widget<Button> ;

    private:
        bool is_hovered_ = false ;
        bool is_pressed_ = false ;
		std::wstring label_ ;
        Font font_ ;
        std::function<void(Canvas*, const Button&)> drawing_logic_ ;
        std::function<void()> callback_ ;

		void UpdateImpl() noexcept {
            if (!drawing_logic_) {
				return ;
			}

            if (!IsValid()) {
				return ;
			}

            drawing_logic_(canvas_.get(), *this) ;
        }

    public:
        Button(const RectF& bound, const Font& font, const std::wstring& label = L"") noexcept : label_(label), font_(font) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;

            SetDrawingLogic([](Canvas* canvas, const Button& button) {
                Renderer render ;
                if (!render.Begin(*canvas)) {
					return ;
				}

				render.Clear(Transparent) ;
                Color button_color ;
                Color border_color ;
                
                if (button.IsPressed()) {
                    button_color = rgba(70, 130, 180, 1) ;
                    border_color = rgba(50, 100, 150, 1) ;
                } else if (button.IsHovered()) {
                    button_color = rgba(100, 149, 237, 1) ;
                    border_color = rgba(70, 119, 207, 1) ;
                } else {
                    button_color = rgba(135, 206, 250, 1) ;
                    border_color = rgba(100, 171, 220, 1) ;
                }
                
                RectF rect = button.GetRelativeBound() ;
                render.FillRectRounded(rect, button_color, 5.0f) ;
                render.DrawRectRounded(rect, border_color, 5.0f, 2.0f) ;
                
                if (!button.GetLabel().empty()) {
                    Color text_color = rgba(255, 255, 255, 1) ;
                    render.DrawString(
						button.GetLabel(), 
						button.GetFont().GetStringBound(button.GetLabel(), {0, button.GetFont().GetAscent()}).AnchorTo(button.GetRelativeBound(), Pivot::Center), 
						text_color, 
						button.GetFont()) ;
                }

                render.End() ;
            }) ;
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                update_ = true ;
            }
            return state ;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (bound_.Contain(mouse_pos)) {
                is_pressed_ = true ;
                update_ = true ;
                return true ;
            }
            return false ;
        }

        bool OnRelease(const PointF& mouse_pos) noexcept {
            bool was_pressed = is_pressed_ ;
            is_pressed_ = false ;
            if (was_pressed) {
                update_ = true ;
                if (bound_.Contain(mouse_pos) && callback_) {
                    callback_() ;
                }
                return bound_.Contain(mouse_pos) ;
            }
            return false ;
        }

        void SetDrawingLogic(std::function<void(Canvas*, const Button&)> drawing_logic) noexcept {
            drawing_logic_ = std::move(drawing_logic) ;
            update_ = true ;
        }
        
        void SetCallback(std::function<void()> callback) noexcept {
            callback_ = std::move(callback) ;
        }
        
        void SetLabel(const std::wstring& label) noexcept {
            if (label_ != label) {
                label_ = label ;
                update_ = true ;
            }
        }

		void SetFont(const Font& font) noexcept { 
			font_ = font ; 
			update_ = true ;
		}

        RectF GetRelativeBound() const noexcept { return {0, 0, bound_.w, bound_.h} ; }
        const std::wstring& GetLabel() const noexcept { return label_ ; }
        const Font& GetFont() const noexcept { return font_ ; }
		const Font* GetFontPtr() const noexcept { return &font_ ; }

        bool IsHovered() const noexcept { return is_hovered_ ; }
        bool IsPressed() const noexcept { return is_pressed_ ; }
    } ;
}