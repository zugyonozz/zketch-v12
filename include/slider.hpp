<<<<<<< HEAD
#pragma once
#include "widget.hpp"

namespace zketch {
	class Slider : public Widget<Slider> {
		friend class Widget<Slider> ;

    public:
        enum Orientation : uint8_t {
            Vertical,
            Horizontal
        } ;

    private:
        Orientation orientation_ ;
        RectF thumb_bound_ ;
        float value_ = 0.0f ;
        float min_value_ = 0.0f ;
        float max_value_ = 100.0f ;
        float offset_ = 0.0f ;
        bool is_dragging_ = false ;
        bool is_hovered_ = false ;
		std::unique_ptr<Canvas> thumb_canvas_ ;
        bool thumb_needs_update_ = true ;
        std::function<void(Canvas*, const Slider&)> drawing_logic_ ;
        
        void UpdateValueFromThumb() noexcept {
            float range = GetMaxValue() ;
            if (range <= 0.0f) {
                value_ = min_value_ ;
                return ;
            }
            
            float normalized ;
            if (orientation_ == Horizontal) {
                normalized = (thumb_bound_.x - bound_.x) / range ;
            } else {
                normalized = (thumb_bound_.y - bound_.y) / range ;
            }
            
            value_ = min_value_ + normalized * (max_value_ - min_value_) ;
            value_ = std::clamp(value_, min_value_, max_value_) ;
        }
        
        void UpdateThumbFromValue() noexcept {
            float range = max_value_ - min_value_ ;
            if (range <= 0.0f) {
				return ;
			}
            
            float normalized = (value_ - min_value_) / range ;
            float max_offset = GetMaxValue() ;
            
            if (orientation_ == Horizontal) {
                thumb_bound_.x = bound_.x + normalized * max_offset ;
            } else {
                thumb_bound_.y = bound_.y + normalized * max_offset ;
            }
        }

        void RenderThumb() noexcept {
            if (!thumb_canvas_ || !thumb_canvas_->IsValid()) {
                return ;
            }

            Renderer render ;
            if (!render.Begin(*thumb_canvas_)) {
                return ;
            }

            render.Clear(Transparent) ;
            Color thumb_color = is_hovered_ || is_dragging_ ? 
                rgba(135, 206, 250, 255) : rgba(100, 149, 237, 255) ;
            
            float radius = std::min(thumb_bound_.w, thumb_bound_.h) / 2.0f ;
            render.FillRectRounded({PointF{}, thumb_bound_.GetSize()}, thumb_color, radius) ;
            render.End() ;

            thumb_needs_update_ = false ;
        }

        void RenderTrack() noexcept {
            if (!canvas_ || !canvas_->IsValid()) {
                return ;
            }

            Renderer render ;
            if (!render.Begin(*canvas_)) {
                return ;
            }

            render.Clear(Transparent) ;
            
            Color track_color = rgba(220, 220, 220, 255) ;
            render.FillRect(GetRelativeTrackBound(), track_color) ;
            
            render.End() ;
        }

		void UpdateImpl() noexcept {
            if (!IsValid() || !thumb_canvas_ || !thumb_canvas_->IsValid()) {
				return ;
			}

            if (drawing_logic_) {
                drawing_logic_(canvas_.get(), *this) ;
                return ;
            }

            if (thumb_needs_update_) {
                RenderThumb() ;
            }

            if (update_) {
                RenderTrack() ;
            }

            Renderer render ;
            if (render.Begin(*canvas_)) {
                if (!update_ && thumb_needs_update_) {
                    RenderTrack() ;
                }
                
                if (update_) {
                    Color track_color = rgba(220, 220, 220, 255) ;
                    render.FillRect(GetRelativeTrackBound(), track_color) ;
                }

                render.DrawCanvas(thumb_canvas_.get(), GetRelativeThumbBound().GetPos()) ;
                render.End() ;
            }

            update_ = false ;
        }

    public:
        Slider(Orientation orientation, const RectF& track, const SizeF& thumb) noexcept : 
		orientation_(orientation), 
		thumb_canvas_(std::make_unique<Canvas>()) {

			bound_ = track ;
            if (orientation_ == Vertical) {
                thumb_bound_ = RectF{
                    PointF{track.x + (track.w - thumb.x) / 2.0f, track.y}, 
                    thumb
                } ;
            } else {
                thumb_bound_ = RectF{
                    PointF{track.x, track.y + (track.h - thumb.y) / 2.0f}, 
                    thumb
                } ;
            }
            
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
			thumb_canvas_->Create(thumb) ;

            RenderThumb() ;
            RenderTrack() ;
            update_ = true ;
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = thumb_bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                thumb_needs_update_ = true ;
                update_ = true ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Hover, value_, this)) ;
            }
            return state ;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (thumb_bound_.Contain(mouse_pos)) {
                is_dragging_ = true ;
                offset_ = orientation_ == Vertical ? 
                    mouse_pos.y - thumb_bound_.y : 
                    mouse_pos.x - thumb_bound_.x ;
                thumb_needs_update_ = true ;
                update_ = true ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_, this)) ;
                return true ;
            }
            
            if (bound_.Contain(mouse_pos)) {
                if (orientation_ == Horizontal) {
                    thumb_bound_.x = std::clamp(
                        mouse_pos.x - thumb_bound_.w / 2.0f,
                        bound_.x,
                        bound_.x + bound_.w - thumb_bound_.w
                    ) ;
                } else {
                    thumb_bound_.y = std::clamp(
                        mouse_pos.y - thumb_bound_.h / 2.0f,
                        bound_.y,
                        bound_.y + bound_.h - thumb_bound_.h
                    ) ;
                }
                UpdateValueFromThumb() ;
                is_dragging_ = true ;
                offset_ = orientation_ == Vertical ? thumb_bound_.h / 2.0f : thumb_bound_.w / 2.0f ;
                thumb_needs_update_ = true ;
                update_ = true ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_, this)) ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_, this)) ;
                return true ;
            }
            
            return false ;
        }

        bool OnRelease() noexcept {
            if (is_dragging_) {
                is_dragging_ = false ;
                thumb_needs_update_ = true ;
                update_ = true ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_, this)) ;
                return true ;
            }
            return false ;
        }

        bool OnDrag(const PointF& mouse_pos) noexcept {
            if (!is_dragging_) return false ;

            float new_pos ;
            if (orientation_ == Horizontal) {
                new_pos = mouse_pos.x - offset_ ;
                thumb_bound_.x = std::clamp(
                    new_pos, 
                    bound_.x, 
                    bound_.x + bound_.w - thumb_bound_.w
                ) ;
            } else {
                new_pos = mouse_pos.y - offset_ ;
                thumb_bound_.y = std::clamp(
                    new_pos, 
                    bound_.y, 
                    bound_.y + bound_.h - thumb_bound_.h
                ) ;
            }
            UpdateValueFromThumb() ;
            update_ = true ;
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_, this)) ;
            return true ;
        }

        void SetDrawingLogic(std::function<void(Canvas*, const Slider&)> drawing_logic) noexcept {
            drawing_logic_ = std::move(drawing_logic) ;
            update_ = true ;
            thumb_needs_update_ = true ;
        }
        
        void SetValue(float value) noexcept {
            float old_value = value_ ;
            value_ = std::clamp(value, min_value_, max_value_) ;
            if (old_value != value_) {
                UpdateThumbFromValue() ;
                update_ = true ;
            }
        }
        
        void SetRange(float min_val, float max_val) noexcept {
            min_value_ = min_val ;
            max_value_ = max_val ;
            value_ = std::clamp(value_, min_value_, max_value_) ;
            UpdateThumbFromValue() ;
            update_ = true ;
        }

		bool IsHovered() const noexcept { return is_hovered_ ; }
        bool IsDragging() const noexcept { return is_dragging_ ; }

		RectF GetRelativeTrackBound() const noexcept { return {0, 0, bound_.w, bound_.h} ; }
        RectF GetRelativeThumbBound() const noexcept { return {thumb_bound_.x - bound_.x, thumb_bound_.y - bound_.y, thumb_bound_.w, thumb_bound_.h } ; }
        float GetValue() const noexcept { return value_ ; }
        float GetMaxValue() const noexcept { return orientation_ == Vertical ? bound_.h - thumb_bound_.h : bound_.w - thumb_bound_.w ; }
        float GetMinValue() const noexcept { return min_value_ ; }
        float GetMaxValueRange() const noexcept { return max_value_ ; }
		Canvas* GetThumbCanvas() const noexcept { return thumb_canvas_.get() ; }
    } ;
=======
#pragma once
#include "widget.hpp"

namespace zketch {
	class Slider : public Widget<Slider> {
		friend class Widget<Slider> ;

    public:
        enum Orientation : uint8_t {
            Vertical,
            Horizontal
        } ;

    private:
        Orientation orientation_ ;
        RectF track_bound_ ;
        RectF thumb_bound_ ;
        float value_ = 0.0f ;
        float min_value_ = 0.0f ;
        float max_value_ = 100.0f ;
        float offset_ = 0.0f ;
        bool is_dragging_ = false ;
        bool is_hovered_ = false ;
        std::function<void(Canvas*, const Slider&)> drawer_ ;
        
        void UpdateValueFromThumb() noexcept {
            float range = GetMaxValue() ;
            if (range <= 0.0f) {
                value_ = min_value_ ;
                return ;
            }
            
            float normalized ;
            if (orientation_ == Horizontal) {
                normalized = (thumb_bound_.x - track_bound_.x) / range ;
            } else {
                normalized = (thumb_bound_.y - track_bound_.y) / range ;
            }
            
            value_ = min_value_ + normalized * (max_value_ - min_value_) ;
            value_ = std::clamp(value_, min_value_, max_value_) ;
        }
        
        void UpdateThumbFromValue() noexcept {
            float range = max_value_ - min_value_ ;
            if (range <= 0.0f) {
				return ;
			}
            
            float normalized = (value_ - min_value_) / range ;
            float max_offset = GetMaxValue() ;
            
            if (orientation_ == Horizontal) {
                thumb_bound_.x = track_bound_.x + normalized * max_offset ;
            } else {
                thumb_bound_.y = track_bound_.y + normalized * max_offset ;
            }
        }

		void UpdateImpl() noexcept {
            if (!drawer_) {
				return ;
			}

            if (!ValidateCanvas("Slider::UpdateImpl()")) {
				return ;
			}

            drawer_(canvas_.get(), *this) ;
        }

    public:
        Slider(Orientation orientation, const RectF& track, const SizeF& thumb) noexcept : orientation_(orientation), track_bound_(track) {
            
            bound_ = track_bound_ ;
            
            if (orientation_ == Vertical) {
                thumb_bound_ = RectF{
                    PointF{track.x + (track.w - thumb.x) / 2.0f, track.y}, 
                    thumb
                } ;
            } else {
                thumb_bound_ = RectF{
                    PointF{track.x, track.y + (track.h - thumb.y) / 2.0f}, 
                    thumb
                } ;
            }
            
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
            
            // FIX: Set transparent clear color
            canvas_->SetClearColor(rgba(0, 0, 0, 0)) ;

            SetDrawer([](Canvas* canvas, const Slider& slider) {
                Drawer drawer ;
                if (!drawer.Begin(*canvas)) return ;

                // No need to call Clear() - already done by Begin()

                Color track_color = rgba(220, 220, 220, 255) ;
                Color thumb_color = slider.IsHovered() || slider.IsDragging() 
                    ? rgba(135, 206, 250, 255) 
                    : rgba(100, 149, 237, 255) ;

                drawer.FillRect(slider.GetRelativeTrackBound(), track_color) ;
                
                // Rounded thumb
                RectF thumb = slider.GetRelativeThumbBound() ;
                float radius = std::min(thumb.w, thumb.h) / 2.0f ;
                drawer.FillRectRounded(thumb, thumb_color, radius) ;

                drawer.End() ;
            }) ;
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = thumb_bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                MarkDirty() ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Hover, value_)) ;
            }
            return state ;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (thumb_bound_.Contain(mouse_pos)) {
                is_dragging_ = true ;
                
                offset_ = orientation_ == Vertical 
                    ? mouse_pos.y - thumb_bound_.y 
                    : mouse_pos.x - thumb_bound_.x ;
                
                MarkDirty() ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_)) ;
                return true ;
            }
            
            // Click on track to jump
            if (track_bound_.Contain(mouse_pos)) {
                if (orientation_ == Horizontal) {
                    thumb_bound_.x = std::clamp(
                        mouse_pos.x - thumb_bound_.w / 2.0f,
                        track_bound_.x,
                        track_bound_.x + track_bound_.w - thumb_bound_.w
                    ) ;
                } else {
                    thumb_bound_.y = std::clamp(
                        mouse_pos.y - thumb_bound_.h / 2.0f,
                        track_bound_.y,
                        track_bound_.y + track_bound_.h - thumb_bound_.h
                    ) ;
                }
                UpdateValueFromThumb() ;
                is_dragging_ = true ;
                offset_ = orientation_ == Vertical 
                    ? thumb_bound_.h / 2.0f 
                    : thumb_bound_.w / 2.0f ;
                MarkDirty() ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_)) ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_)) ;
                return true ;
            }
            
            return false ;
        }

        bool OnRelease() noexcept {
            if (is_dragging_) {
                is_dragging_ = false ;
                MarkDirty() ;
                EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_)) ;
                return true ;
            }
            return false ;
        }

        bool OnDrag(const PointF& mouse_pos) noexcept {
            if (!is_dragging_) return false ;

            float new_pos ;
            if (orientation_ == Horizontal) {
                new_pos = mouse_pos.x - offset_ ;
                thumb_bound_.x = std::clamp(
                    new_pos, 
                    track_bound_.x, 
                    track_bound_.x + track_bound_.w - thumb_bound_.w
                ) ;
            } else {
                new_pos = mouse_pos.y - offset_ ;
                thumb_bound_.y = std::clamp(
                    new_pos, 
                    track_bound_.y, 
                    track_bound_.y + track_bound_.h - thumb_bound_.h
                ) ;
            }
            UpdateValueFromThumb() ;
            MarkDirty() ;
            EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_)) ;
            return true ;
        }

        void PresentImpl(HWND hwnd) noexcept {
            if (!ValidateCanvas("Slider::PresentImpl()")) return ;
            canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
        }

        void SetDrawer(std::function<void(Canvas*, const Slider&)> drawer) noexcept {
            drawer_ = std::move(drawer) ;
            MarkDirty() ;
        }
        
        void SetValue(float value) noexcept {
            value_ = std::clamp(value, min_value_, max_value_) ;
            UpdateThumbFromValue() ;
            MarkDirty() ;
        }
        
        void SetRange(float min_val, float max_val) noexcept {
            min_value_ = min_val ;
            max_value_ = max_val ;
            value_ = std::clamp(value_, min_value_, max_value_) ;
            UpdateThumbFromValue() ;
            MarkDirty() ;
        }
        
        RectF GetRelativeTrackBound() const noexcept {
            return {0, 0, bound_.w, bound_.h} ;
        }

        RectF GetRelativeThumbBound() const noexcept {
            return {
                thumb_bound_.x - bound_.x,
                thumb_bound_.y - bound_.y,
                thumb_bound_.w,
                thumb_bound_.h
            } ;
        }

        float GetValue() const noexcept { 
			return value_ ; 
		}

        float GetMaxValue() const noexcept { 
            return orientation_ == Vertical 
                ? track_bound_.h - thumb_bound_.h 
                : track_bound_.w - thumb_bound_.w ;
        }

        float GetMinValue() const noexcept { 
			return min_value_ ; 
		}

        float GetMaxValueRange() const noexcept { 
			return max_value_ ; 
		}

        bool IsHovered() const noexcept { 
			return is_hovered_ ; 
		}

        bool IsDragging() const noexcept { 
			return is_dragging_ ; 
		}
    } ;
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}