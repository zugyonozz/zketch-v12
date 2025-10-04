#pragma once
#include "widget.hpp"

namespace zketch {

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
    std::function<void(Renderer*, const Slider&)> drawer_;
    
    // Cache untuk menghindari rekalkulasi
    RectF last_thumb_bound_;
    
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
        if (range <= 0.0f) {
            return;
        }
        
        float normalized = (value_ - min_value_) / range;
        float max_offset = GetMaxValue();
        
        if (orientation_ == Horizontal) {
            thumb_bound_.x = track_bound_.x + normalized * max_offset;
        } else {
            thumb_bound_.y = track_bound_.y + normalized * max_offset;
        }
    }
    
    void UpdateImpl() noexcept {
        if (!drawer_ || !ValidateCanvas("Slider::UpdateImpl()")) {
            return;
        }
        
        Renderer renderer;
        bool full_redraw = NeedsFullRedraw();
        
        if (!renderer.Begin(*canvas_, full_redraw)) {
            logger::error("Slider::UpdateImpl - Failed to begin renderer");
            return;
        }
        
        drawer_(&renderer, *this);
        renderer.End();
        
        // Cache untuk optimasi berikutnya
        last_thumb_bound_ = thumb_bound_;
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
        
        last_thumb_bound_ = thumb_bound_;
        
        canvas_ = std::make_unique<Canvas>();
        if (!canvas_->Create(bound_.GetSize())) {
            logger::error("Slider - Failed to create canvas");
            return;
        }
        
        canvas_->SetClearColor(rgba(0, 0, 0, 0));
        
        SetDrawer([](Renderer* renderer, const Slider& slider) {
            Color track_color = rgba(220, 220, 220, 255);
            Color thumb_color = slider.IsHovered() || slider.IsDragging()
                ? rgba(135, 206, 250, 255)
                : rgba(100, 149, 237, 255);
            
            renderer->FillRect(slider.GetRelativeTrackBound(), track_color);
            
            RectF thumb = slider.GetRelativeThumbBound();
            float radius = std::min(thumb.w, thumb.h) / 2.0f;
            renderer->FillRectRounded(thumb, thumb_color, radius);
        });
    }
    
    bool OnHover(const PointF& mouse_pos) noexcept {
        if (!enabled_) return false;
        
        bool state = thumb_bound_.Contain(mouse_pos);
        if (state != is_hovered_) {
            is_hovered_ = state;
            
            // Invalidate hanya thumb region
            InvalidateRegion(GetRelativeThumbBound());
            
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Hover, value_));
        }
        return state;
    }
    
    bool OnPress(const PointF& mouse_pos) noexcept {
        if (!enabled_) return false;
        
        if (thumb_bound_.Contain(mouse_pos)) {
            is_dragging_ = true;
            
            offset_ = orientation_ == Vertical
                ? mouse_pos.y - thumb_bound_.y
                : mouse_pos.x - thumb_bound_.x;
            
            InvalidateRegion(GetRelativeThumbBound());
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_));
            return true;
        }
        
        // Click on track to jump
        if (track_bound_.Contain(mouse_pos)) {
            // Invalidate old thumb position
            InvalidateRegion(GetRelativeThumbBound());
            
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
            
            // Invalidate new thumb position
            InvalidateRegion(GetRelativeThumbBound());
            
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_));
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_));
            return true;
        }
        
        return false;
    }
    
    bool OnRelease() noexcept {
        if (is_dragging_) {
            is_dragging_ = false;
            InvalidateRegion(GetRelativeThumbBound());
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_));
            return true;
        }
        return false;
    }
    
    bool OnDrag(const PointF& mouse_pos) noexcept {
        if (!is_dragging_) return false;
        
        // Invalidate old position
        RectF old_thumb = GetRelativeThumbBound();
        
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
        
        // Invalidate union of old and new position
        RectF new_thumb = GetRelativeThumbBound();
        RectF invalidate_region = {
            std::min(old_thumb.x, new_thumb.x),
            std::min(old_thumb.y, new_thumb.y),
            std::max(old_thumb.x + old_thumb.w, new_thumb.x + new_thumb.w) - std::min(old_thumb.x, new_thumb.x),
            std::max(old_thumb.y + old_thumb.h, new_thumb.y + new_thumb.h) - std::min(old_thumb.y, new_thumb.y)
        };
        
        InvalidateRegion(invalidate_region);
        
        EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_));
        return true;
    }
    
    void PresentImpl(HWND hwnd) noexcept {
        if (!ValidateCanvas("Slider::PresentImpl()")) return;
        canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)});
    }
    
    void SetDrawer(std::function<void(Renderer*, const Slider&)> drawer) noexcept {
        drawer_ = std::move(drawer);
        MarkDirty(true);
    }
    
    void SetValue(float value) noexcept {
        float new_value = std::clamp(value, min_value_, max_value_);
        if (new_value != value_) {
            // Invalidate old thumb position
            InvalidateRegion(GetRelativeThumbBound());
            
            value_ = new_value;
            UpdateThumbFromValue();
            
            // Invalidate new thumb position
            InvalidateRegion(GetRelativeThumbBound());
        }
    }
    
    void SetRange(float min_val, float max_val) noexcept {
        min_value_ = min_val;
        max_value_ = max_val;
        value_ = std::clamp(value_, min_value_, max_value_);
        UpdateThumbFromValue();
        MarkDirty(true); // Full redraw for range change
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
    
    float GetValue() const noexcept {
        return value_;
    }
    
    float GetMaxValue() const noexcept {
        return orientation_ == Vertical
            ? track_bound_.h - thumb_bound_.h
            : track_bound_.w - thumb_bound_.w;
    }
    
    float GetMinValue() const noexcept {
        return min_value_;
    }
    
    float GetMaxValueRange() const noexcept {
        return max_value_;
    }
    
    bool IsHovered() const noexcept {
        return is_hovered_;
    }
    
    bool IsDragging() const noexcept {
        return is_dragging_;
    }
};

} // namespace zketch