#pragma once
#include "font.hpp"

namespace zketch {

	class Canvas {
		friend class Renderer ;
		friend class Window ;

	private :
		std::unique_ptr<Gdiplus::Bitmap> front_ {} ;
		std::unique_ptr<Gdiplus::Bitmap> back_ {} ;
		Size size_ {} ;
		bool invalidate_ ;


	public :
		Canvas(const Canvas&) = delete ;
		Canvas& operator=(const Canvas&) = delete ;
		Canvas(Canvas&&) = default ;
		Canvas& operator=(Canvas&&) = default ;
		Canvas() = default ;
		~Canvas() = default ;

		bool Create(const Size& size) noexcept {
			Clear() ;

			#ifdef CANVAS_DEBUG
				logger::info("Canvas::Create - Creating GDI+ bitmap: ", size.x, " x ", size.y, '.') ;
			#endif

			try {
				front_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
				back_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
			} catch (...) {
				front_.reset() ;
				back_.reset() ;

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Exception while creating bitmap.") ;
				#endif

				return false ;
			}

			if (!front_ || !back_) {

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Failed to allocate bitmap.") ;
				#endif

				return false ;
			}

			Gdiplus::Status fst = front_->GetLastStatus() ;
			Gdiplus::Status bst = back_->GetLastStatus() ;

			#ifdef CANVAS_DEBUG
				logger::info("Canvas::Create - front buffer status: ", static_cast<int32_t>(fst)) ;
				logger::info("Canvas::Create - back buffer status: ", static_cast<int32_t>(bst)) ;
			#endif

			if (fst != Gdiplus::Ok || bst != Gdiplus::Ok) {
				front_.reset() ;
				back_.reset() ;

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Failed to create front buffer, status: ", static_cast<int32_t>(fst)) ;
					logger::error("Canvas::Create - Failed to create back buffer, status: ", static_cast<int32_t>(bst)) ;
				#endif

				return false ;
			}

			{
				Gdiplus::Graphics gfx_front(front_.get()) ;
				Gdiplus::Graphics gfx_back(back_.get()) ;
				gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
				gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
			}

			size_ = size ;
			invalidate_ = true ;
			return true ;
		}

		void Clear() noexcept {
			front_.reset() ;
			front_.reset() ;
			size_ = {} ;
			invalidate_ = false ;

			#ifdef CANVAS_DEBUG
				logger::info("Canvas::Clear - Canvas cleared.") ;
			#endif
		}

		void SwapBuffers() noexcept {
			if (front_ && back_) {
				std::swap(front_, back_) ;
				invalidate_ = false ;
			}
		}

		bool IsValid() const noexcept { return front_ != nullptr && back_ != nullptr ; }
		bool Invalidate() const noexcept { return invalidate_ ; }
		void MarkInvalidate() noexcept { invalidate_ = true ; }
		void MarkValidate() noexcept { invalidate_ = false ; }

		Gdiplus::Bitmap* GetBitmap() const noexcept { return front_.get() ; }
		Gdiplus::Bitmap* GetBackBuffer() const noexcept { return back_.get() ; }
		uint32_t GetWidth() const noexcept { return size_.x ; }
		uint32_t GetHeight() const noexcept { return size_.y ; }
		const Size& GetSize() const noexcept { return size_ ; }
	} ;

	struct CanvasAttachment {
		Canvas* canvas_ = nullptr ;
		PointF pos_ = {} ;
	} ;

}