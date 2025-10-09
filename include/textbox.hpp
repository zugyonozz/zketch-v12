<<<<<<< HEAD
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
=======
#pragma once 
#include "widget.hpp"

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

                // No need to call Clear() - already done by Begin() with bg_color_

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
            canvas_->SetClearColor(color) ; // Update canvas clear color
            MarkDirty() ;
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("TextBox::PresentImpl()")) {
				return ;
			}

            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
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
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}