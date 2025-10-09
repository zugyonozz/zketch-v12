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
}