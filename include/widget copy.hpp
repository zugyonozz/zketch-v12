#pragma once
#include "window copy.hpp"

namespace zketch {
	template <typename Derived>
    class Widget {
    protected:
        std::unique_ptr<Canvas> canvas_ ;
        RectF bound_ ;
        bool needs_redraw_ = true ;
        bool visible_ = true ;
        
        bool ValidateCanvas(const char* context) const noexcept {
            if (!canvas_ || !canvas_->IsValid()) {
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
        
        void Update() noexcept { 
            if (needs_redraw_ && visible_) {
                static_cast<Derived*>(this)->UpdateImpl() ;
                needs_redraw_ = false ;
            }
        }

		Canvas* GetCanvas() noexcept {
			if (!visible_) {
				return nullptr ;
			}
            Update() ;
			return canvas_.get() ;
		}
        
        void Present(Window& window) noexcept { 
            if (!visible_) return ;
            Update() ;
            static_cast<Derived*>(this)->PresentImpl(window) ;
        }
        
        void MarkDirty() noexcept { 
			needs_redraw_ = true ; 
		}

        bool NeedsRedraw() const noexcept { 
			return needs_redraw_ ; 
		}
        
        void SetVisible(bool visible) noexcept { 
            visible_ = visible ; 
            if (visible) MarkDirty() ;
        }

        bool IsVisible() const noexcept { 
			return visible_ ; 
		}
        
        void SetPosition(const PointF& pos) noexcept {
            bound_.x = pos.x ;
            bound_.y = pos.y ;
            MarkDirty() ;
        }

		void SetClearColor(const Color& color) noexcept {
			if (canvas_) {
				canvas_->SetClearColor(color) ;
				MarkDirty() ;
			}
		}
    } ;
}