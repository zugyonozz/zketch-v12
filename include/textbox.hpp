#pragma once 
#include "widget.hpp"

namespace zketch {
	class TextBox : public Widget<TextBox> {
		friend class Widget<TextBox> ;
		
    private:
        std::wstring text_ ;
        Font font_ ;
        std::function<void(Canvas*, const TextBox&)> drawing_logic_ ;

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
        TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept : text_(text), font_(font) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
            
            SetDrawingLogic([](Canvas* canvas, const TextBox& textbox) {
                Renderer render ;
                if (!render.Begin(*canvas)) {
                    return ;
                }

				render.Clear(Transparent) ;
                
                render.FillRectRounded(
                    textbox.GetRelativeBound(), 
                    Red,
                    3.0f
                ) ;
                
                render.DrawRectRounded(
                    textbox.GetRelativeBound(),
                    White,
                    3.0f,
                    1.0f
                ) ;
                
                render.DrawString(
                    textbox.GetText(), 
                    textbox.GetFont().GetStringBound(textbox.GetText(), {0, textbox.GetFont().GetAscent()}).AnchorTo(textbox.GetRelativeBound(), Pivot::Center),
                    Black, 
                    textbox.GetFont()
                ) ;

                render.End() ;
            }) ;
        }

        void SetText(const std::wstring_view& text) noexcept {
            if (text_ != text) {
                text_ = text ;
                update_ = true ;
            }
        }

        void SetFont(const Font& font) noexcept {
            font_ = font ;
            update_ = true ;
        }
        
        void SetDrawingLogic(std::function<void(Canvas*, const TextBox&)> drawing_logic) noexcept {
            drawing_logic_ = std::move(drawing_logic) ;
            update_ = true ;
        }

        RectF GetRelativeBound() const noexcept { return {0, 0, bound_.w, bound_.h} ; }
        const std::wstring& GetText() const noexcept { return text_ ; }
        const Font& GetFont() const noexcept { return font_ ; }
    } ;
}