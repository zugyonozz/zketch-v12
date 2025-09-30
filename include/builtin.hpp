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
        RectF bound_;
        bool needs_redraw_ = true;
        bool visible_ = true;
        
        bool ValidateCanvas(const char* context) const noexcept {
            if (!canvas_ || !canvas_->IsValid()) {
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
            if (needs_redraw_ && visible_) {
                static_cast<Derived*>(this)->UpdateImpl();
                needs_redraw_ = false;
            }
        }
        
        void Present(HWND hwnd) noexcept { 
            if (!visible_) return;
            Update();
            static_cast<Derived*>(this)->PresentImpl(hwnd);
        }
        
        void MarkDirty() noexcept { needs_redraw_ = true; }
        bool NeedsRedraw() const noexcept { return needs_redraw_; }
        
        void SetVisible(bool visible) noexcept { 
            visible_ = visible; 
            if (visible) MarkDirty();
        }
        bool IsVisible() const noexcept { return visible_; }
        
        void SetPosition(const PointF& pos) noexcept {
            bound_.x = pos.x;
            bound_.y = pos.y;
            MarkDirty();
        }
    };

    // ============================================
    // Slider Class - Optimized
    // ============================================
    class Slider : public Widget<Slider> {
		friend class Widget<Slider>;

    public:
        enum Orientation : uint8_t {
            Vertical,
            Horizontal
        };

    private:
        Orientation orientation_;
        RectF track_bound_;
        RectF thumb_bound_;
        float value_ = 0.0f;
        float min_value_ = 0.0f;
        float max_value_ = 100.0f;
        float offset_ = 0.0f;
        bool is_dragging_ = false;
        bool is_hovered_ = false;
        std::function<void(Canvas*, const Slider&)> drawer_;
        
        void UpdateValueFromThumb() noexcept {
            float range = GetMaxValue();
            if (range <= 0.0f) {
                value_ = min_value_;
                return;
            }
            
            float normalized;
            if (orientation_ == Horizontal) {
                normalized = (thumb_bound_.x - track_bound_.x) / range;
            } else {
                normalized = (thumb_bound_.y - track_bound_.y) / range;
            }
            
            value_ = min_value_ + normalized * (max_value_ - min_value_);
            value_ = std::clamp(value_, min_value_, max_value_);
        }
        
        void UpdateThumbFromValue() noexcept {
            float range = max_value_ - min_value_;
            if (range <= 0.0f) return;
            
            float normalized = (value_ - min_value_) / range;
            float max_offset = GetMaxValue();
            
            if (orientation_ == Horizontal) {
                thumb_bound_.x = track_bound_.x + normalized * max_offset;
            } else {
                thumb_bound_.y = track_bound_.y + normalized * max_offset;
            }
        }

		void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("Slider::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
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
                
                // Rounded thumb
                RectF thumb = slider.GetRelativeThumbBound();
                float radius = std::min(thumb.w, thumb.h) / 2.0f;
                drawer.FillRectRounded(thumb, thumb_color, radius);

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
            
            // Click on track to jump
            if (track_bound_.Contain(mouse_pos)) {
                if (orientation_ == Horizontal) {
                    thumb_bound_.x = std::clamp(
                        mouse_pos.x - thumb_bound_.w / 2.0f,
                        track_bound_.x,
                        track_bound_.x + track_bound_.w - thumb_bound_.w
                    );
                } else {
                    thumb_bound_.y = std::clamp(
                        mouse_pos.y - thumb_bound_.h / 2.0f,
                        track_bound_.y,
                        track_bound_.y + track_bound_.h - thumb_bound_.h
                    );
                }
                UpdateValueFromThumb();
                is_dragging_ = true;
                offset_ = orientation_ == Vertical 
                    ? thumb_bound_.h / 2.0f 
                    : thumb_bound_.w / 2.0f;
                MarkDirty();
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_));
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_));
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
                thumb_bound_.x = std::clamp(
                    new_pos, 
                    track_bound_.x, 
                    track_bound_.x + track_bound_.w - thumb_bound_.w
                );
            } else {
                new_pos = mouse_pos.y - offset_;
                thumb_bound_.y = std::clamp(
                    new_pos, 
                    track_bound_.y, 
                    track_bound_.y + track_bound_.h - thumb_bound_.h
                );
            }
            UpdateValueFromThumb();
            MarkDirty();
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_));
            return true;
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("Slider::PresentImpl()")) return;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
        }

        void SetDrawer(std::function<void(Canvas*, const Slider&)> drawer) noexcept {
            drawer_ = std::move(drawer);
            MarkDirty();
        }
        
        void SetValue(float value) noexcept {
            value_ = std::clamp(value, min_value_, max_value_);
            UpdateThumbFromValue();
            MarkDirty();
        }
        
        void SetRange(float min_val, float max_val) noexcept {
            min_value_ = min_val;
            max_value_ = max_val;
            value_ = std::clamp(value_, min_value_, max_value_);
            UpdateThumbFromValue();
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
        float GetMinValue() const noexcept { return min_value_; }
        float GetMaxValueRange() const noexcept { return max_value_; }
        bool IsHovered() const noexcept { return is_hovered_; }
        bool IsDragging() const noexcept { return is_dragging_; }
    };

    // ============================================
    // Button Class - Optimized
    // ============================================
    class Button : public Widget<Button> {
		friend class Widget<Button> ;

    private:
        bool is_hovered_ = false;
        bool is_pressed_ = false;
        std::wstring label_;
        Font font_;
        std::function<void(Canvas*, const Button&)> drawer_;
        std::function<void()> on_click_;

		void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("Button::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
        }

    public:
        Button(const RectF& bound, const std::wstring& label = L"", const Font& font = Font()) noexcept 
            : label_(label), font_(font) {
            bound_ = bound;
            canvas_ = std::make_unique<Canvas>();
            canvas_->Create(bound_.getSize());

            SetDrawer([](Canvas* canvas, const Button& button) {
                Drawer drawer;
                if (!drawer.Begin(*canvas)) return;

                Color bg_color = rgba(250, 250, 250, 0);
                Color button_color;
                Color border_color;
                
                if (button.IsPressed()) {
                    button_color = rgba(70, 130, 180, 255);
                    border_color = rgba(50, 100, 150, 255);
                } else if (button.IsHovered()) {
                    button_color = rgba(100, 149, 237, 255);
                    border_color = rgba(70, 119, 207, 255);
                } else {
                    button_color = rgba(135, 206, 250, 255);
                    border_color = rgba(100, 171, 220, 255);
                }

                drawer.Clear(bg_color);
                
                RectF rect = button.GetRelativeBound();
                drawer.FillRectRounded(rect, button_color, 5.0f);
                drawer.DrawRectRounded(rect, border_color, 5.0f, 2.0f);
                
                if (!button.GetLabel().empty()) {
                    Color text_color = rgba(255, 255, 255, 255);
                    Point text_pos = {
                        static_cast<int32_t>(rect.w / 2 - 30),
                        static_cast<int32_t>(rect.h / 2 - 10)
                    };
                    drawer.DrawString(button.GetLabel(), text_pos, text_color, button.GetFont());
                }

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
                if (bound_.Contain(mouse_pos) && on_click_) {
                    on_click_();
                }
                return bound_.Contain(mouse_pos);
            }
            return false;
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("Button::PresentImpl()")) return;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
        }

        void SetDrawer(std::function<void(Canvas*, const Button&)> drawer) noexcept {
            drawer_ = std::move(drawer);
            MarkDirty();
        }
        
        void SetOnClick(std::function<void()> callback) noexcept {
            on_click_ = std::move(callback);
        }
        
        void SetLabel(const std::wstring& label) noexcept {
            if (label_ != label) {
                label_ = label;
                MarkDirty();
            }
        }

        RectF GetRelativeBound() const noexcept {
            return {0, 0, bound_.w, bound_.h};
        }

        const std::wstring& GetLabel() const noexcept { return label_; }
        const Font& GetFont() const noexcept { return font_; }
        bool IsHovered() const noexcept { return is_hovered_; }
        bool IsPressed() const noexcept { return is_pressed_; }
    };
    
    // ============================================
    // TextBox Class - Optimized
    // ============================================
    class TextBox : public Widget<TextBox> {
		friend class Widget<TextBox>;
    private:
        std::wstring text_;
        Font font_;
        Color text_color_ = rgba(50, 50, 50, 255);
        Color bg_color_ = rgba(250, 250, 250, 255);
        std::function<void(Canvas*, const TextBox&)> drawer_;

		void UpdateImpl() noexcept {
            if (!drawer_) return;
            if (!ValidateCanvas("TextBox::UpdateImpl()")) return;
            drawer_(canvas_.get(), *this);
        }

    public:
        TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept
            : text_(text), font_(font) {
            bound_ = bound;
            canvas_ = std::make_unique<Canvas>();
            canvas_->Create(bound_.getSize());
            
            SetDrawer([](Canvas* canvas, const TextBox& textbox){
                Drawer drawer;
                if (!drawer.Begin(*canvas)) return;

                drawer.Clear(textbox.GetBackgroundColor());
                drawer.DrawString(
                    textbox.GetText(), 
                    {5, 5}, 
                    textbox.GetTextColor(), 
                    textbox.GetFont()
                );

                drawer.End();
            });
        }

        void SetText(const std::wstring& text) noexcept {
            if (text_ != text) {
                text_ = text;
                MarkDirty();
            }
        }
        
        void SetTextColor(const Color& color) noexcept {
            text_color_ = color;
            MarkDirty();
        }
        
        void SetBackgroundColor(const Color& color) noexcept {
            bg_color_ = color;
            MarkDirty();
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
        const Color& GetTextColor() const noexcept { return text_color_; }
        const Color& GetBackgroundColor() const noexcept { return bg_color_; }
    };

}