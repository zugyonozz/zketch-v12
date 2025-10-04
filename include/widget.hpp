#pragma once
#include "renderer.hpp"
#include "event.hpp"

namespace zketch {
    template <typename Derived>
    class Widget {
    protected:
        std::unique_ptr<Canvas> canvas_ ;
        RectF bound_ ;
        bool needs_redraw_ = true ;
        bool visible_ = true ;
        bool enabled_ = true ;
        
        bool ValidateCanvas(const char* context) const noexcept {
            if (!canvas_) {
                logger::warning(context, " - Canvas is null") ;
                return false ;
            }
            
            if (!canvas_->IsValid()) {
                logger::warning(context, " - Canvas is invalid") ;
                return false ;
            }
            
            return true ;
        }
        
    public:
        Widget() noexcept = default ;
        virtual ~Widget() noexcept = default ;
        
        const RectF& GetBound() const noexcept { 
            return bound_ ; 
        }

        const Canvas* GetCanvas() const noexcept { 
            return canvas_.get() ; 
        }
        
        Canvas* GetCanvas() noexcept { 
            return canvas_.get() ; 
        }
        
        void Update() noexcept { 
            if (needs_redraw_ && visible_) {
                static_cast<Derived*>(this)->UpdateImpl() ;
                needs_redraw_ = false ;
            }
        }
        
        void Present(HWND hwnd) noexcept { 
            if (!visible_) return ;
            Update() ;
            static_cast<Derived*>(this)->PresentImpl(hwnd) ;
        }
        
        void MarkDirty() noexcept { 
            needs_redraw_ = true ; 
        }

        bool NeedsRedraw() const noexcept { 
            return needs_redraw_ ; 
        }
        
        void SetVisible(bool visible) noexcept { 
            if (visible_ != visible) {
                visible_ = visible ; 
                if (visible) MarkDirty() ;
            }
        }

        bool IsVisible() const noexcept { 
            return visible_ ; 
        }
        
        void SetEnabled(bool enabled) noexcept {
            enabled_ = enabled ;
        }

        bool IsEnabled() const noexcept {
            return enabled_ ;
        }
        
        void SetPosition(const PointF& pos) noexcept {
            if (bound_.x != pos.x || bound_.y != pos.y) {
                bound_.x = pos.x ;
                bound_.y = pos.y ;
                MarkDirty() ;
            }
        }

        void SetBound(const RectF& bound) noexcept {
            if (bound_ != bound) {
                bound_ = bound ;
                
                // Resize canvas jika ukuran berubah
                if (canvas_ && canvas_->GetSize() != bound.GetSize()) {
                    canvas_->Resize(bound.GetSize()) ;
                }
                
                MarkDirty() ;
            }
        }

        void SetClearColor(const Color& color) noexcept {
            if (canvas_) {
                canvas_->SetClearColor(color) ;
                MarkDirty() ;
            }
        }

        Color GetClearColor() const noexcept {
            return canvas_ ? canvas_->GetClearColor() : Transparent ;
        }

        // Hit test - cek apakah point ada di dalam widget
        bool HitTest(const PointF& point) const noexcept {
            return bound_.Contain(point) ;
        }

        // Convert global coordinates ke local widget coordinates
        PointF GlobalToLocal(const PointF& global_pos) const noexcept {
            return {global_pos.x - bound_.x, global_pos.y - bound_.y} ;
        }

        // Convert local coordinates ke global
        PointF LocalToGlobal(const PointF& local_pos) const noexcept {
            return {local_pos.x + bound_.x, local_pos.y + bound_.y} ;
        }
    } ;
}