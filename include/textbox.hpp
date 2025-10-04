#pragma once
#include "widget.hpp"

namespace zketch {

class TextBox : public Widget<TextBox> {
    friend class Widget<TextBox>;

private:
    std::wstring text_;
    Font font_;
    Color text_color_ = rgba(50, 50, 50, 255);
    Color bg_color_ = rgba(250, 250, 250, 255);
    std::function<void(Renderer*, const TextBox&)> drawer_;
    
    // Cache text measurements untuk optimasi
    RectF text_bounds_;
    bool text_bounds_valid_ = false;
    
    void UpdateImpl() noexcept {
        if (!drawer_ || !ValidateCanvas("TextBox::UpdateImpl()")) {
            return;
        }
        
        Renderer renderer;
        bool full_redraw = NeedsFullRedraw();
        
        if (!renderer.Begin(*canvas_, full_redraw)) {
            logger::error("TextBox::UpdateImpl - Failed to begin renderer");
            return;
        }
        
        drawer_(&renderer, *this);
        renderer.End();
        
        text_bounds_valid_ = false; // Reset setelah redraw
    }
    
    RectF MeasureTextBounds() const {
        if (text_bounds_valid_) {
            return text_bounds_;
        }
        
        if (text_.empty() || !canvas_ || !canvas_->IsValid()) {
            return RectF{0, 0, 0, 0};
        }
        
        // Approximate measurement (exact measurement memerlukan Graphics object)
        float approx_width = text_.length() * font_.GetSize() * 0.6f;
        float approx_height = font_.GetSize() * 1.2f;
        
        return RectF{5, 5, approx_width, approx_height};
    }

public:
    TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept
        : text_(text), font_(font) {
        
        bound_ = bound;
        canvas_ = std::make_unique<Canvas>();
        
        if (!canvas_->Create(bound_.GetSize())) {
            logger::error("TextBox - Failed to create canvas");
            return;
        }
        
        canvas_->SetClearColor(bg_color_);
        
        SetDrawer([](Renderer* renderer, const TextBox& textbox) {
            // Background sudah di-clear oleh Renderer::Begin()
            renderer->DrawString(
                textbox.GetText(),
                {5, 5},
                textbox.GetTextColor(),
                textbox.GetFont()
            );
        });
    }
    
    void SetText(const std::wstring& text) noexcept {
        if (text_ != text) {
            // Invalidate old text region
            if (!text_.empty() && text_bounds_valid_) {
                InvalidateRegion(text_bounds_);
            }
            
            text_ = text;
            text_bounds_valid_ = false;
            
            // Invalidate new text region
            text_bounds_ = MeasureTextBounds();
            text_bounds_valid_ = true;
            InvalidateRegion(text_bounds_);
        }
    }
    
    void AppendText(const std::wstring& text) noexcept {
        if (!text.empty()) {
            // Invalidate old bounds
            if (text_bounds_valid_) {
                InvalidateRegion(text_bounds_);
            }
            
            text_ += text;
            text_bounds_valid_ = false;
            
            // Invalidate new bounds
            text_bounds_ = MeasureTextBounds();
            text_bounds_valid_ = true;
            InvalidateRegion(text_bounds_);
        }
    }
    
    void Clear() noexcept {
        if (!text_.empty()) {
            // Invalidate old text region
            if (text_bounds_valid_) {
                InvalidateRegion(text_bounds_);
            }
            
            text_.clear();
            text_bounds_valid_ = false;
            text_bounds_ = RectF{0, 0, 0, 0};
        }
    }
    
    void SetTextColor(const Color& color) noexcept {
        if (text_color_.ABGR != color.ABGR) {
            text_color_ = color;
            
            // Partial invalidate untuk text area saja
            if (text_bounds_valid_) {
                InvalidateRegion(text_bounds_);
            } else {
                MarkDirty();
            }
        }
    }
    
    void SetBackgroundColor(const Color& color) noexcept {
        if (bg_color_.ABGR != color.ABGR) {
            bg_color_ = color;
            canvas_->SetClearColor(color);
            MarkDirty(true); // Full redraw untuk background
        }
    }
    
    void SetFont(const Font& font) noexcept {
        font_ = font;
        text_bounds_valid_ = false;
        
        // Invalidate old dan new text bounds
        if (!text_.empty()) {
            text_bounds_ = MeasureTextBounds();
            text_bounds_valid_ = true;
            InvalidateRegion(text_bounds_);
        }
    }
    
    void PresentImpl(HWND hwnd) noexcept {
        if (!ValidateCanvas("TextBox::PresentImpl()")) {
            return;
        }
        
        canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
    }
    
    void SetDrawer(std::function<void(Renderer*, const TextBox&)> drawer) noexcept {
        drawer_ = std::move(drawer);
        MarkDirty(true);
    }
    
    const std::wstring& GetText() const noexcept {
        return text_;
    }
    
    const Font& GetFont() const noexcept {
        return font_;
    }
    
    const Color& GetTextColor() const noexcept {
        return text_color_;
    }
    
    const Color& GetBackgroundColor() const noexcept {
        return bg_color_;
    }
    
    size_t GetTextLength() const noexcept {
        return text_.length();
    }
    
    bool IsEmpty() const noexcept {
        return text_.empty();
    }
};

} // namespace zketch