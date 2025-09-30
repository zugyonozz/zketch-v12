#pragma once
#include "window.hpp"

namespace zketch {
    
    // ============================================
    // Base Widget Class - CRTP Pattern
    // ============================================
    template <typename Derived>
    class Widget {
    protected:
        std::unique_ptr<Canvas> canvas_;
        RectF bound_; // Koordinat absolut di layar
        bool needs_redraw_ = true;
        
        bool ValidateCanvas(const char* context) const noexcept {
            if (!canvas_ || !canvas_->IsValid()) {
                // logger::warning(context, " - Canvas not valid");
                return false;
            }
            return true;
        }
        
    public:
        Widget() noexcept = default;
        virtual ~Widget() noexcept = default;
        
        const RectF& GetBound() const noexcept { return bound_; }
        const Canvas* GetCanvas() const noexcept { return canvas_.get(); }
        
        void Update() noexcept { 
            if (needs_redraw_) {
                static_cast<Derived*>(this)->UpdateImpl();
                needs_redraw_ = false;
            }
        }
        
        void Present(HWND hwnd) noexcept { 
            Update(); // Pastikan tergambar sebelum ditampilkan
            static_cast<Derived*>(this)->PresentImpl(hwnd);
        }
        
        void MarkDirty() noexcept { needs_redraw_ = true; }
        bool NeedsRedraw() const noexcept { return needs_redraw_; }
    };

    // ============================================
    // Slider Class - DIPERBAIKI DENGAN DRAWER KUSTOM
    // ============================================
    class Slider : public Widget<Slider> {
    public:
        enum Orientation : uint8_t {
            Vertical,
            Horizontal
        };

    private:
        Orientation orientation_;
        RectF track_bound_; // Absolut
        RectF thumb_bound_; // Absolut
        float value_ = 0.0f;
        float offset_ = 0.0f;
        bool is_dragging_ = false;
        bool is_hovered_ = false;
        std::function<void(Canvas*, const Slider&)> drawer_;
        
        void UpdateValueFromThumb() noexcept {
            if (orientation_ == Horizontal) {
                value_ = thumb_bound_.x - track_bound_.x;
            } else {
                value_ = thumb_bound_.y - track_bound_.y;
            }
             value_ = std::clamp(value_, 0.0f, GetMaxValue());
        }

    public:
        Slider(Orientation orientation, const RectF& track, const SizeF& thumb) noexcept 
            : orientation_(orientation), track_bound_(track) {
            
            bound_ = track_bound_;
            
            if (orientation_ == Vertical) {
                thumb_bound_ = RectF{
                    PointF{track.x + (track.w - thumb.x) / 2.0f, track.y}, 
                    thumb
                };
            } else {
                thumb_bound_ = RectF{
                    PointF{track.x, track.y + (track.h - thumb.y) / 2.0f}, 
                    thumb
                };
            }
            
            canvas_ = std::make_unique<Canvas>();
            canvas_->Create(bound_.getSize());

            SetDrawer([](Canvas* canvas, const Slider& slider) {
                Drawer drawer;
                if (!drawer.Begin(*canvas)) return;

                Color bg_color = rgba(250, 250, 250, 0);
                Color track_color = rgba(220, 220, 220, 255);
                Color thumb_color = slider.IsHovered() || slider.IsDragging() 
                    ? rgba(135, 206, 250, 255) 
                    : rgba(100, 149, 237, 255);

                drawer.Clear(bg_color);
                drawer.FillRect(slider.GetRelativeTrackBound(), track_color);
                drawer.FillRect(slider.GetRelativeThumbBound(), thumb_color);

                drawer.End();
            });
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = thumb_bound_.Contain(mouse_pos);
            if (state != is_hovered_) {
                is_hovered_ = state;
                MarkDirty();
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Hover, value_));
            }
            return state;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (thumb_bound_.Contain(mouse_pos)) {
                is_dragging_ = true;
                
                offset_ = orientation_ == Vertical 
                    ? mouse_pos.y - thumb_bound_.y 
                    : mouse_pos.x - thumb_bound_.x;
                
                MarkDirty();
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_));
                return true;
            }
            return false;
        }

        bool OnRelease() noexcept {
            if (is_dragging_) {
                is_dragging_ = false;
                MarkDirty();
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_));
                return true;
            }
            return false;
        }

        bool OnDrag(const PointF& mouse_pos) noexcept {
            if (!is_dragging_) return false;

            float new_pos;
            if (orientation_ == Horizontal) {
                new_pos = mouse_pos.x - offset_;
                thumb_bound_.x = std::clamp(new_pos, track_bound_.x, track_bound_.x + track_bound_.w - thumb_bound_.w);
            } else {
                new_pos = mouse_pos.y - offset_;
                thumb_bound_.y = std::clamp(new_pos, track_bound_.y, track_bound_.y + track_bound_.h - thumb_bound_.h);
            }
            UpdateValueFromThumb();
            MarkDirty();
			EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_));
            return true;
        }

        void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("Slider::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("Slider::PresentImpl()")) return;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
        }

        void SetDrawer(std::function<void(Canvas*, const Slider&)> drawer) noexcept {
            drawer_ = std::move(drawer);
            MarkDirty();
        }
        
        RectF GetRelativeTrackBound() const noexcept {
            return {0, 0, bound_.w, bound_.h};
        }

        RectF GetRelativeThumbBound() const noexcept {
            return {
                thumb_bound_.x - bound_.x,
                thumb_bound_.y - bound_.y,
                thumb_bound_.w,
                thumb_bound_.h
            };
        }

        float GetValue() const noexcept { return value_; }
        float GetMaxValue() const noexcept { 
            return orientation_ == Vertical 
                ? track_bound_.h - thumb_bound_.h 
                : track_bound_.w - thumb_bound_.w;
        }
        bool IsHovered() const noexcept { return is_hovered_; }
        bool IsDragging() const noexcept { return is_dragging_; }
    };

    // ============================================
    // Button Class - DENGAN DRAWER KUSTOM
    // ============================================
    class Button : public Widget<Button> {
    private:
        bool is_hovered_ = false;
        bool is_pressed_ = false;
        std::function<void(Canvas*, const Button&)> drawer_;

    public:
        Button(const RectF& bound) noexcept {
            bound_ = bound;
            canvas_ = std::make_unique<Canvas>();
            canvas_->Create(bound_.getSize());

            SetDrawer([](Canvas* canvas, const Button& button) {
                Drawer drawer;
                if (!drawer.Begin(*canvas)) return;

                Color bg_color = rgba(250, 250, 250, 0); // Transparan
                Color button_color;
                if (button.IsPressed()) {
                    button_color = rgba(180, 180, 180, 255); // Pressed
                } else if (button.IsHovered()) {
                    button_color = rgba(220, 220, 220, 255); // Hover
                } else {
                    button_color = rgba(240, 240, 240, 255); // Normal
                }

                drawer.Clear(bg_color);
                drawer.FillRect(button.GetRelativeBound(), button_color);
                // Tambahkan border jika perlu
                drawer.DrawRect(button.GetRelativeBound(), rgba(150, 150, 150, 255), 1.0f);

                drawer.End();
            });
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = bound_.Contain(mouse_pos);
            if (state != is_hovered_) {
                is_hovered_ = state;
                MarkDirty();
            }
            return state;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (bound_.Contain(mouse_pos)) {
                is_pressed_ = true;
                MarkDirty();
                return true;
            }
            return false;
        }

        bool OnRelease(const PointF& mouse_pos) noexcept {
            bool was_pressed = is_pressed_;
            is_pressed_ = false;
            if (was_pressed) {
                MarkDirty();
                // Return true jika pelepasan terjadi di dalam bound
                return bound_.Contain(mouse_pos);
            }
            return false;
        }

        void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("Button::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("Button::PresentImpl()")) return;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
        }

        void SetDrawer(std::function<void(Canvas*, const Button&)> drawer) noexcept {
            drawer_ = std::move(drawer);
            MarkDirty();
        }

        RectF GetRelativeBound() const noexcept {
            return {0, 0, bound_.w, bound_.h};
        }

        bool IsHovered() const noexcept { return is_hovered_; }
        bool IsPressed() const noexcept { return is_pressed_; }
    };
    
    // ============================================
    // TextBox Class - DENGAN DRAWER KUSTOM
    // ============================================
    class TextBox : public Widget<TextBox> {
    private:
        std::wstring text_;
        Font font_; // Asumsi kelas Font ada
        std::function<void(Canvas*, const TextBox&)> drawer_;

    public:
        TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept
            : text_(text), font_(font) {
            bound_ = bound;
            canvas_ = std::make_unique<Canvas>();
            canvas_->Create(bound_.getSize());
            
            SetDrawer([](Canvas* canvas, const TextBox& textbox){
                Drawer drawer;
                if (!drawer.Begin(*canvas)) return;

                Color bg_color = rgba(250, 250, 250, 0); // Transparan
                Color text_color = rgba(50, 50, 50, 255); // Hitam

                drawer.Clear(bg_color);
                // Gambar text di pojok kiri atas (0,0) kanvas relatif
                drawer.DrawString(textbox.GetText(), {0, 0}, text_color, textbox.GetFont());

                drawer.End();
            });
        }

        void SetText(const std::wstring& text) {
            if (text_ != text) {
                text_ = text;
                MarkDirty();
            }
        }
        
        void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("TextBox::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("TextBox::PresentImpl()")) return;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
        }
        
        void SetDrawer(std::function<void(Canvas*, const TextBox&)> drawer) noexcept {
            drawer_ = std::move(drawer);
            MarkDirty();
        }

        const std::wstring& GetText() const noexcept { return text_; }
        const Font& GetFont() const noexcept { return font_; }
    };

}