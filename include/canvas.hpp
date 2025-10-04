#pragma once 
#include "font.hpp"

namespace zketch {
	class Canvas {
		friend class Renderer ;

	private :

		// static member
		static std::unique_ptr<Gdiplus::Bitmap> front_buffer_ ;
		static std::unique_ptr<Gdiplus::Bitmap> back_buffer_ ;
		static Size canvas_size_ ;

		// non-static member
		RectF bound_ = {} ;
		Rect invalidate_bound_ = {} ;
		bool invalidate_ = false ;

		bool Create(const Size& size) noexcept {
			try {
				front_buffer_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
				back_buffer_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
			} catch (...) {
				front_buffer_.reset() ;
				back_buffer_.reset() ;
				logger::error("Exception while creating bitmap.") ;
				return false ;
			}

			if (!front_buffer_ || !back_buffer_) {
				logger::error("Failed to allocate bitmap.") ;
				return false ;
			}

			Gdiplus::Status fst = front_buffer_->GetLastStatus() ;
			Gdiplus::Status bst = back_buffer_->GetLastStatus() ;
			logger::info("front buffer status: ", static_cast<int32_t>(fst)) ;
			logger::info("back buffer status: ", static_cast<int32_t>(bst)) ;
			if (fst != Gdiplus::Ok || bst != Gdiplus::Ok) {
				front_buffer_.reset() ;
				back_buffer_.reset() ;
				logger::error("Failed to create front buffer, status: ", static_cast<int32_t>(fst)) ;
				logger::error("Failed to create back buffer, status: ", static_cast<int32_t>(bst)) ;
				return false ;
			}

			{
				Gdiplus::Graphics gfx_front(front_buffer_.get()) ;
				Gdiplus::Graphics gfx_back(back_buffer_.get()) ;
				gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
				gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
			}

			canvas_size_ = size ;
			return true ;
		}

	public :
		Canvas() noexcept = default ;
		Canvas(const Canvas&) = delete ;
		Canvas(Canvas&&) = default ;
		Canvas& operator=(const Canvas&) = delete ;
		Canvas& operator=(Canvas&&) = default ;
		~Canvas() noexcept = default ;

		Canvas(const RectF& bound) noexcept : bound_(bound) {
			Rect r {Point{}, canvas_size_} ;
			if (r == r.Union(bound_)) {
				return ;
			} else {
				Create(r.GetSize()) ;
			}
		}

		bool operator!() const noexcept {
			return IsCanvasValid() ;
		}

		void Clear() noexcept {
			front_buffer_.reset() ;
			back_buffer_.reset() ;
			logger::info("Canvas cleared.") ;
		}

		void Present(HWND hwnd) noexcept {
			if (!front_buffer_) {
				logger::warning("Canvas::Present - No bitmap.") ;
				return ;
			}

			if (!hwnd) {
				logger::warning("Canvas::Present - hwnd is null.") ;
				return ;
			}

			HDC hdc = GetDC(hwnd) ;
			if (!hdc) {
				logger::warning("Canvas::Present - GetDC failed.") ;
				return ;
			}

			Gdiplus::Graphics screen(hdc) ;
			screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			screen.DrawImage(front_buffer_.get(), 0, 0) ;

			ReleaseDC(hwnd, hdc) ;
		}

		void Validate() noexcept {
			invalidate_bound_ = {} ;
			invalidate_ = false ;
		}

		void Invalidate(const Rect& rect = {}) noexcept {
			if (rect != Rect{}) {
				invalidate_bound_ = rect ;
			}
			invalidate_ = true ;
		}

		bool IsCanvasValid() const noexcept { 
			return front_buffer_ && back_buffer_ && bound_.GetSize() == Size{} ; 
		}

		Size GetSize() const noexcept {
			return canvas_size_ ;
		}

		uint32_t GetWidth() const noexcept {
			return canvas_size_.x ;
		}

		uint32_t GetHeight() const noexcept {
			return canvas_size_.y ;
		}

		Gdiplus::Bitmap* GetFrontBuffer() const noexcept {
			return front_buffer_.get() ;
		}

		Gdiplus::Bitmap* GetBackBuffer() const noexcept {
			return back_buffer_.get() ;
		}

		void SwapBuffers() noexcept {
			if (front_buffer_ && back_buffer_) {
				std::swap(front_buffer_, back_buffer_) ;
				invalidate_ = false ;
			}
		}
	} ;
}