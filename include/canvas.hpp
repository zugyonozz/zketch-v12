#pragma once 
#include "font.hpp"

namespace zketch {
    class Canvas {
        friend class Renderer ;

    private:
        // STATIC: Shared buffer untuk SEMUA canvas (mencegah flickering)
        static std::unique_ptr<Gdiplus::Bitmap> front_buffer_ ;
        static std::unique_ptr<Gdiplus::Bitmap> back_buffer_ ;
        static Size canvas_size_ ;
        static std::mutex buffer_mutex_ ; // Thread safety

        // NON-STATIC: Setiap canvas punya bound sendiri
        RectF bound_ = {} ;
        Rect invalidate_bound_ = {} ;
        bool invalidate_ = false ;
        Color clear_color_ = rgba(0, 0, 0, 0) ;

        // Helper: Check if static buffer perlu expand
        bool NeedsExpansion(const Size& required_size) const noexcept {
            if (canvas_size_.x == 0 || canvas_size_.y == 0) {
                return true ; // Belum diinisialisasi
            }
            
            return required_size.x > canvas_size_.x || 
                   required_size.y > canvas_size_.y ;
        }

        // Helper: Expand static buffer untuk accommodate all canvas
        bool ExpandBuffer(const Size& new_size) noexcept {
            std::lock_guard<std::mutex> lock(buffer_mutex_) ;

            Size expanded = {
                std::max(canvas_size_.x, new_size.x),
                std::max(canvas_size_.y, new_size.y)
            } ;

            if (expanded.x == 0 || expanded.y == 0) {
                logger::error("Canvas::ExpandBuffer - Invalid size") ;
                return false ;
            }

            logger::info("Expanding buffer from ", canvas_size_.x, "x", canvas_size_.y, 
                        " to ", expanded.x, "x", expanded.y) ;

            try {
                auto new_front = std::make_unique<Gdiplus::Bitmap>(
                    expanded.x, expanded.y, PixelFormat32bppARGB
                ) ;
                auto new_back = std::make_unique<Gdiplus::Bitmap>(
                    expanded.x, expanded.y, PixelFormat32bppARGB
                ) ;

                if (!new_front || !new_back) {
                    logger::error("Canvas::ExpandBuffer - Allocation failed") ;
                    return false ;
                }

                if (new_front->GetLastStatus() != Gdiplus::Ok || 
                    new_back->GetLastStatus() != Gdiplus::Ok) {
                    logger::error("Canvas::ExpandBuffer - Bitmap creation failed") ;
                    return false ;
                }

                // Copy old content jika ada
                if (front_buffer_ && back_buffer_) {
                    Gdiplus::Graphics gfx_front(new_front.get()) ;
                    Gdiplus::Graphics gfx_back(new_back.get()) ;
                    
                    gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
                    gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
                    
                    gfx_front.DrawImage(front_buffer_.get(), 0, 0) ;
                    gfx_back.DrawImage(back_buffer_.get(), 0, 0) ;
                } else {
                    Gdiplus::Graphics gfx_front(new_front.get()) ;
                    Gdiplus::Graphics gfx_back(new_back.get()) ;
                    gfx_front.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
                    gfx_back.Clear(Gdiplus::Color(0, 0, 0, 0)) ;
                }

                front_buffer_ = std::move(new_front) ;
                back_buffer_ = std::move(new_back) ;
                canvas_size_ = expanded ;

                return true ;
            } catch (const std::exception& e) {
                logger::error("Canvas::ExpandBuffer - Exception: ", e.what()) ;
                return false ;
            } catch (...) {
                logger::error("Canvas::ExpandBuffer - Unknown exception") ;
                return false ;
            }
        }

    public:
        Canvas() noexcept = default ;
        Canvas(const Canvas&) = delete ;
        Canvas(Canvas&&) noexcept = default ;
        Canvas& operator=(const Canvas&) = delete ;
        Canvas& operator=(Canvas&&) noexcept = default ;
        ~Canvas() noexcept = default ;

        Canvas(const RectF& bound) noexcept : bound_(bound) {
            Create(bound_.GetSize()) ;
        }

        bool Create(const Size& size) noexcept {
            if (size.x == 0 || size.y == 0) {
                logger::error("Canvas::Create - Invalid size: ", size.x, "x", size.y) ;
                return false ;
            }

            // Set bound untuk canvas ini
            bound_ = RectF{0, 0, static_cast<float>(size.x), static_cast<float>(size.y)} ;

            // Check if static buffer perlu expand
            if (NeedsExpansion(size)) {
                if (!ExpandBuffer(size)) {
                    return false ;
                }
            }

            logger::info("Canvas created with bound: ", size.x, "x", size.y) ;
            return true ;
        }

        void Clear() noexcept {
            // Tidak clear static buffer, hanya reset bound
            bound_ = {} ;
            logger::info("Canvas bound cleared") ;
        }

        static void ClearStaticBuffers() noexcept {
            std::lock_guard<std::mutex> lock(buffer_mutex_) ;
            front_buffer_.reset() ;
            back_buffer_.reset() ;
            canvas_size_ = {} ;
            logger::info("Static buffers cleared") ;
        }

        void SetClearColor(const Color& color) noexcept {
            clear_color_ = color ;
        }

        Color GetClearColor() const noexcept {
            return clear_color_ ;
        }

        void Present(HWND hwnd, const Point& offset = {0, 0}) noexcept {
            std::lock_guard<std::mutex> lock(buffer_mutex_) ;

            if (!IsValid()) {
                logger::warning("Canvas::Present - Invalid canvas") ;
                return ;
            }

            if (!hwnd) {
                logger::warning("Canvas::Present - Invalid window handle") ;
                return ;
            }

            HDC hdc = GetDC(hwnd) ;
            if (!hdc) {
                logger::warning("Canvas::Present - GetDC failed") ;
                return ;
            }

            Gdiplus::Graphics screen(hdc) ;
            screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
            screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
            screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
            
            // Draw ONLY this canvas's region dari shared buffer
            screen.DrawImage(
                front_buffer_.get(), 
                offset.x, 
                offset.y,
                static_cast<int>(bound_.x),
                static_cast<int>(bound_.y),
                static_cast<int>(bound_.w),
                static_cast<int>(bound_.h),
                Gdiplus::UnitPixel
            ) ;

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

        bool IsValid() const noexcept { 
            return front_buffer_ && back_buffer_ && 
                   canvas_size_.x > 0 && canvas_size_.y > 0 &&
                   bound_.w > 0 && bound_.h > 0 ; // FIX: Check bound juga
        }

        bool IsInvalidated() const noexcept {
            return invalidate_ ;
        }

        Size GetSize() const noexcept {
            return bound_.GetSize() ; // Return bound size, bukan static size
        }

        uint32_t GetWidth() const noexcept {
            return static_cast<uint32_t>(bound_.w) ;
        }

        uint32_t GetHeight() const noexcept {
            return static_cast<uint32_t>(bound_.h) ;
        }

        const RectF& GetBound() const noexcept {
            return bound_ ;
        }

        void SetBound(const RectF& bound) noexcept {
            bound_ = bound ;
            
            // Check if static buffer perlu expand
            Size required = bound.GetSize() ;
            if (NeedsExpansion(required)) {
                ExpandBuffer(required) ;
            }
        }

        Gdiplus::Bitmap* GetFrontBuffer() const noexcept {
            return front_buffer_.get() ;
        }

        Gdiplus::Bitmap* GetBackBuffer() const noexcept {
            return back_buffer_.get() ;
        }

        void SwapBuffers() noexcept {
            std::lock_guard<std::mutex> lock(buffer_mutex_) ;
            if (front_buffer_ && back_buffer_) {
                std::swap(front_buffer_, back_buffer_) ;
                invalidate_ = false ;
            }
        }

        // Get static buffer size
        static Size GetStaticBufferSize() noexcept {
            return canvas_size_ ;
        }

        // Expand canvas bound untuk fit new area
        bool ExpandToFit(const RectF& new_bound) noexcept {
            Rect current = {
                Point{static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)},
                Size{static_cast<uint32_t>(bound_.w), static_cast<uint32_t>(bound_.h)}
            } ;
            
            Rect target = {
                Point{static_cast<int32_t>(new_bound.x), static_cast<int32_t>(new_bound.y)},
                Size{static_cast<uint32_t>(new_bound.w), static_cast<uint32_t>(new_bound.h)}
            } ;

            if (current.w == 0 || current.h == 0) {
                // First initialization
                bound_ = new_bound ;
                return Create(new_bound.GetSize()) ;
            }

            Rect expanded = current.Union(target) ;
            
            if (expanded == current) {
                // No expansion needed
                return true ;
            }

            // Update bound
            bound_ = RectF{
                static_cast<float>(expanded.x),
                static_cast<float>(expanded.y),
                static_cast<float>(expanded.w),
                static_cast<float>(expanded.h)
            } ;

            // Expand static buffer if needed
            if (NeedsExpansion(expanded.GetSize())) {
                return ExpandBuffer(expanded.GetSize()) ;
            }

            return true ;
        }

        operator bool() const noexcept {
            return IsValid() ;
        }
    } ;

    // Static member initialization
    std::unique_ptr<Gdiplus::Bitmap> Canvas::front_buffer_ = nullptr ;
    std::unique_ptr<Gdiplus::Bitmap> Canvas::back_buffer_ = nullptr ;
    Size Canvas::canvas_size_ = {} ;
    std::mutex Canvas::buffer_mutex_ ;
}