<<<<<<< HEAD
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
=======
#pragma once 
#include "font.hpp"

namespace zketch {
	class Canvas {
		friend class Drawer ;

	private:
		std::unique_ptr<Gdiplus::Bitmap> front_{} ;
		std::unique_ptr<Gdiplus::Bitmap> back_{} ;
		Size size_{} ;
		bool dirty_ = false ;
		Color clear_color_ = rgba(0, 0, 0, 0); // Default transparent

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

			// Clear both buffers initially
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

		void Present(HWND hwnd) noexcept {
			if (!front_) {
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
			screen.DrawImage(front_.get(), 0, 0) ;

			ReleaseDC(hwnd, hdc) ;
		}

		void Present(HWND hwnd, const Point& pos) const noexcept {
			if (!front_) {
				logger::warning("Canvas::Present - No bitmap.") ;
				return ;
			}

			if (!hwnd) {
				logger::warning("Canvas::Present - hwnd is null.") ;
				return ;
			}

			HDC hdc = GetDC(hwnd) ;
			if (!hdc) {
				logger::warning("Canvas::Present - invalid HDC.") ;
				return ;
			}

			Gdiplus::Graphics screen(hdc) ;
			screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			screen.DrawImage(front_.get(), pos.x, pos.y) ;

			ReleaseDC(hwnd, hdc) ;
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
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}