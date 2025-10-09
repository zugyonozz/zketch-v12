#pragma once
#include "font.hpp"

namespace zketch {

	class Canvas {
		friend class Renderer ;
		friend class Window ;

	private :
		std::unique_ptr<Gdiplus::Bitmap> canvas_ {} ;
		bool invalidate_ = false ;

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
				canvas_ = std::make_unique<Gdiplus::Bitmap>(size.x, size.y, PixelFormat32bppARGB) ;
			} catch (...) {
				canvas_.reset() ;

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Exception while creating bitmap.") ;
				#endif

				return false ;
			}

			if (!canvas_) {

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Failed to allocate bitmap.") ;
				#endif

				return false ;
			}

			Gdiplus::Status status = canvas_->GetLastStatus() ;

			#ifdef CANVAS_DEBUG
				logger::info("Canvas::Create - Buffer status: ", static_cast<int32_t>(status)) ;
			#endif

			if (status != Gdiplus::Ok) {
				canvas_.reset() ;

				#ifdef CANVAS_DEBUG
					logger::error("Canvas::Create - Failed to create buffer, status: ", static_cast<int32_t>(status)) ;
				#endif

				return false ;
			}

			{
				Gdiplus::Graphics gfx_front(canvas_.get()) ;
				gfx_front.Clear(Transparent) ;
			}

			invalidate_ = true ;
			return true ;
		}

		void Clear() noexcept {
			canvas_.reset() ;
			invalidate_ = false ;

			#ifdef CANVAS_DEBUG
				logger::info("Canvas::Clear - Canvas cleared.") ;
			#endif
		}

		bool IsValid() const noexcept { return canvas_ != nullptr ; }
		bool Invalidate() const noexcept { return invalidate_ ; }
		void MarkInvalidate() noexcept { invalidate_ = true ; }
		void MarkValidate() noexcept { invalidate_ = false ; }

		Gdiplus::Bitmap* GetBitmap() const noexcept { return canvas_.get() ; }
		uint32_t GetWidth() const noexcept { return canvas_->GetWidth() ; }
		uint32_t GetHeight() const noexcept { return canvas_->GetHeight() ; }
		Size GetSize() const noexcept { return {GetWidth(), GetHeight()} ; }
	} ;
}