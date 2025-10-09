<<<<<<< HEAD
#pragma once
#include "renderer.hpp"

namespace zketch {
	template <typename Derived>
    class Widget {
    protected:
        std::unique_ptr<Canvas> canvas_ ;
        RectF bound_ ;
        bool update_ = true ;
        bool visible_ = true ;
        
        bool IsValid() const noexcept {
            return canvas_ && canvas_->IsValid() ; 
        }
        
    public:
        Widget() noexcept = default ;
        virtual ~Widget() noexcept = default ;
        
        void InvokeUpdate() noexcept { 
            if (update_ && visible_) {
                static_cast<Derived*>(this)->UpdateImpl() ;
                update_ = false ;
            }
        }
        
        void SetVisible(bool visible) noexcept { 
            visible_ = visible ; 
            if (visible) {
				update_ = true ;
			}
        }
        
        void SetPosition(const PointF& pos) noexcept {
            bound_.x = pos.x ;
            bound_.y = pos.y ;
            update_ = true ;
        }

		PointF GetPosition() const noexcept { return bound_.GetPos() ; }
		const RectF& GetBound() const noexcept { return bound_ ; }

        const Canvas* GetCanvas() const noexcept { 
			if (!visible_) {
				return nullptr ;
			}

			return canvas_.get() ; 
		}

		Canvas* GetCanvas() noexcept {
			if (!visible_) {
				return nullptr ;
			}
			
			return canvas_.get() ;
		}

		bool IsVisible() const noexcept { return visible_ ; }
		bool IsUpdate() const noexcept { return update_ ; }
    } ;
=======
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
			canvas_->SetClearColor(color) ;
		}
    } ;

	namespace Integration {
	
	}
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}