#pragma once 
#include "widget copy.hpp"

namespace zketch {
	class TextBox : public Widget<TextBox> {
		friend class Widget<TextBox> ;
    private:
        std::wstring text_ ;
        Font font_ ;
        Color text_color_ = rgba(50, 50, 50, 255) ;
        Color bg_color_ = rgba(250, 250, 250, 255) ;
        std::function<void(Canvas*, const TextBox&)> drawer_ ;

		void UpdateImpl() noexcept {
            if (!drawer_) {
				return ;
			}
			
            if (!ValidateCanvas("TextBox::UpdateImpl()")) {
				return ;
			}

            drawer_(canvas_.get(), *this) ;
        }

    public:
        TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept : text_(text), font_(font) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
            canvas_->SetClearColor(bg_color_) ;
            
            SetDrawer([](Canvas* canvas, const TextBox& textbox){
                Drawer drawer ;
                if (!drawer.Begin(*canvas)) return ;

                drawer.DrawString(
                    textbox.GetText(), 
                    {5, 5}, 
                    textbox.GetTextColor(), 
                    textbox.GetFont()
                ) ;

                drawer.End() ;
            }) ;
        }

        void SetText(const std::wstring& text) noexcept {
            if (text_ != text) {
                text_ = text ;
                MarkDirty() ;
            }
        }
        
        void SetTextColor(const Color& color) noexcept {
            text_color_ = color ;
            MarkDirty() ;
        }
        
        void SetBackgroundColor(const Color& color) noexcept {
            bg_color_ = color ;
            if (canvas_) {
                canvas_->SetClearColor(color) ;
            }
            MarkDirty() ;
        }

        void PresentImpl(Window& window) noexcept {
            if (!ValidateCanvas("TextBox::PresentImpl()")) {
				return ;
			}

            window.Present(*canvas_, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
        }
        
        void SetDrawer(std::function<void(Canvas*, const TextBox&)> drawer) noexcept {
            drawer_ = std::move(drawer) ;
            MarkDirty() ;
        }

        const std::wstring& GetText() const noexcept { 
			return text_ ; 
		}

        const Font& GetFont() const noexcept { 
			return font_ ; 
		}

        const Color& GetTextColor() const noexcept { 
			return text_color_ ; 
		}

        const Color& GetBackgroundColor() const noexcept { 
			return bg_color_ ; 
		}
    } ;
}