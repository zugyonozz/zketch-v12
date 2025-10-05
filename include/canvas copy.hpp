#pragma once 
#include "font.hpp"

namespace zketch {
	class Canvas {
		friend class Drawer ;
		friend class Window ;

	private:
		std::unique_ptr<Gdiplus::Bitmap> front_{} ;
		std::unique_ptr<Gdiplus::Bitmap> back_{} ;
		Size size_{} ;
		bool dirty_ = false ;
		Color clear_color_ = rgba(0, 0, 0, 0);

	public:
		Canvas(const Canvas&) = delete ;
		Canvas& operator=(const Canvas&) = delete ;
		Canvas() = default ;
		Canvas(Canvas&&) = default ;
		Canvas& operator=(Canvas&&) = default ;
		~Canvas() = default ;

		bool Create(const Size& size) noexcept {
			Clear() ;

			logger::info("Creating GDI+ bitmap: ", size.x, " x ", size.y) ;
			try {
				front_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
				back_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
			} catch (...) {
				front_.reset() ;
				back_.reset() ;
				logger::error("Exception while creating bitmap.") ;
				return false ;
			}

			if (!front_ || !back_) {
				logger::error("Failed to allocate bitmap.") ;
				return false ;
			}

			Gdiplus::Status fst = front_->GetLastStatus() ;
			Gdiplus::Status bst = back_->GetLastStatus() ;
			logger::info("front buffer status: ", static_cast<int32_t>(fst)) ;
			logger::info("back buffer status: ", static_cast<int32_t>(bst)) ;
			if (fst != Gdiplus::Ok || bst != Gdiplus::Ok) {
				front_.reset() ;
				back_.reset() ;
				logger::error("Failed to create front buffer, status: ", static_cast<int32_t>(fst)) ;
				logger::error("Failed to create back buffer, status: ", static_cast<int32_t>(bst)) ;
				return false ;
			}

			{
				Gdiplus::Graphics gfx_front(front_.get()) ;
				Gdiplus::Graphics gfx_back(back_.get()) ;
				gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
				gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
			}

			size_ = size ;
			dirty_ = true ;
			return true ;
		}

		void Clear() noexcept {
			front_.reset() ;
			back_.reset() ;
			size_ = {} ;
			dirty_ = false ;
			logger::info("Canvas cleared.") ;
		}

		void SetClearColor(const Color& color) noexcept {
			clear_color_ = color;
		}

		const Color& GetClearColor() const noexcept {
			return clear_color_;
		}

		Gdiplus::Bitmap* GetBitmap() const noexcept { 
			return front_.get() ; 
		}

		Gdiplus::Bitmap* GetBackBuffer() const noexcept { 
			return back_.get() ; 
		}

		int32_t GetWidth() const noexcept { 
			return size_.x ; 
		}

		int32_t GetHeight() const noexcept { 
			return size_.y ; 
		}

		const Size& GetSize() const noexcept { 
			return size_ ; 
		}

		bool IsValid() const noexcept { 
			return front_ != nullptr && back_ != nullptr ; 
		}

		bool NeedRedraw() const noexcept { 
			return dirty_ ; 
		}

		void SwapBuffers() noexcept {
			if (front_ && back_) {
				std::swap(front_, back_) ;
				dirty_ = false ;
			}
		}

		void MarkDirty() noexcept { 
			dirty_ = true ; 
		}

		void MarkClean() noexcept {
			dirty_ = false ;
		}
	} ;
}