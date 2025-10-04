#pragma once
#include "bitmap_pool.hpp"
#include "font.hpp"

namespace zketch {

class Canvas {
    friend class Renderer;

private:
    // PER-CANVAS isolated bitmap dari pool
    std::shared_ptr<BitmapPool::BitmapEntry> bitmap_;
    
    // Canvas-specific state
    RectF bound_;
    Rect dirty_rect_; // Tracking invalidation
    bool is_dirty_ = true;
    Color clear_color_ = rgba(0, 0, 0, 0);
    
    // Clipping region untuk partial updates
    std::vector<Rect> clip_regions_;
    
    bool EnsureBitmap(const Size& required_size) {
        if (bitmap_ && 
            bitmap_->size.x >= required_size.x && 
            bitmap_->size.y >= required_size.y) {
            return true;
        }
        
        // Release old bitmap
        if (bitmap_) {
            g_bitmap_pool.Release(bitmap_);
        }
        
        // Acquire new bitmap from pool
        bitmap_ = g_bitmap_pool.Acquire(required_size);
        
        if (!bitmap_ || !bitmap_->IsValid()) {
            logger::error("Canvas: Failed to acquire bitmap from pool");
            return false;
        }
        
        // Mark entire canvas as dirty
        is_dirty_ = true;
        dirty_rect_ = Rect{0, 0, required_size.x, required_size.y};
        
        return true;
    }

public:
    Canvas() noexcept = default;
    
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    
    Canvas(Canvas&& o) noexcept 
        : bitmap_(std::move(o.bitmap_))
        , bound_(o.bound_)
        , dirty_rect_(o.dirty_rect_)
        , is_dirty_(o.is_dirty_)
        , clear_color_(o.clear_color_)
        , clip_regions_(std::move(o.clip_regions_)) 
    {
        o.bound_ = {};
        o.is_dirty_ = false;
    }
    
    Canvas& operator=(Canvas&& o) noexcept {
        if (this != &o) {
            if (bitmap_) {
                g_bitmap_pool.Release(bitmap_);
            }
            
            bitmap_ = std::move(o.bitmap_);
            bound_ = o.bound_;
            dirty_rect_ = o.dirty_rect_;
            is_dirty_ = o.is_dirty_;
            clear_color_ = o.clear_color_;
            clip_regions_ = std::move(o.clip_regions_);
            
            o.bound_ = {};
            o.is_dirty_ = false;
        }
        return *this;
    }
    
    ~Canvas() noexcept {
        if (bitmap_) {
            g_bitmap_pool.Release(bitmap_);
        }
    }
    
    Canvas(const RectF& bound) noexcept : bound_(bound) {
        Create(bound_.GetSize());
    }
    
    bool Create(const Size& size) {
        if (size.x == 0 || size.y == 0) {
            logger::error("Canvas::Create - Invalid size: ", size.x, "x", size.y);
            return false;
        }
        
        bound_ = RectF{0, 0, static_cast<float>(size.x), static_cast<float>(size.y)};
        
        if (!EnsureBitmap(size)) {
            return false;
        }
        
        logger::info("Canvas created: ", size.x, "x", size.y);
        return true;
    }
    
    bool Resize(const Size& new_size) {
        if (new_size.x == 0 || new_size.y == 0) {
            logger::error("Canvas::Resize - Invalid size");
            return false;
        }
        
        // Get old content
        std::unique_ptr<Gdiplus::Bitmap> old_front;
        Size old_size = bound_.GetSize();
        
        if (bitmap_ && bitmap_->front) {
            old_front = std::make_unique<Gdiplus::Bitmap>(
                old_size.x, old_size.y, PixelFormat32bppPARGB
            );
            
            Gdiplus::Graphics gfx(old_front.get());
            gfx.DrawImage(bitmap_->front.get(), 0, 0, old_size.x, old_size.y);
        }
        
        bound_.w = static_cast<float>(new_size.x);
        bound_.h = static_cast<float>(new_size.y);
        
        if (!EnsureBitmap(new_size)) {
            return false;
        }
        
        // Restore old content if possible
        if (old_front && bitmap_) {
            Gdiplus::Graphics gfx_front(bitmap_->front.get());
            Gdiplus::Graphics gfx_back(bitmap_->back.get());
            
            gfx_front.Clear(clear_color_);
            gfx_back.Clear(clear_color_);
            
            uint32_t copy_w = std::min(old_size.x, new_size.x);
            uint32_t copy_h = std::min(old_size.y, new_size.y);
            
            gfx_front.DrawImage(old_front.get(), 0, 0, copy_w, copy_h);
            gfx_back.DrawImage(old_front.get(), 0, 0, copy_w, copy_h);
        }
        
        Invalidate();
        logger::info("Canvas resized to ", new_size.x, "x", new_size.y);
        return true;
    }
    
    void Clear() {
        bound_ = {};
        is_dirty_ = false;
        dirty_rect_ = {};
        clip_regions_.clear();
        
        if (bitmap_) {
            g_bitmap_pool.Release(bitmap_);
            bitmap_.reset();
        }
        
        logger::info("Canvas cleared");
    }
    
    void SetClearColor(const Color& color) {
        clear_color_ = color;
    }
    
    Color GetClearColor() const {
        return clear_color_;
    }
    
    void Present(HWND hwnd, const Point& offset = {0, 0}) {
        if (!IsValid()) {
            logger::warning("Canvas::Present - Invalid canvas");
            return;
        }
        
        if (!hwnd) {
            logger::warning("Canvas::Present - Invalid window handle");
            return;
        }
        
        HDC hdc = GetDC(hwnd);
        if (!hdc) {
            logger::warning("Canvas::Present - GetDC failed");
            return;
        }
        
        Gdiplus::Graphics screen(hdc);
        screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
        screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
        screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        
        // Draw only this canvas's content
        uint32_t w = static_cast<uint32_t>(bound_.w);
        uint32_t h = static_cast<uint32_t>(bound_.h);
        
        if (clip_regions_.empty()) {
            // Full blit
            screen.DrawImage(
                bitmap_->front.get(),
                offset.x,
                offset.y,
                0, 0, w, h,
                Gdiplus::UnitPixel
            );
        } else {
            // Partial blit with clip regions
            for (const auto& clip : clip_regions_) {
                screen.DrawImage(
                    bitmap_->front.get(),
                    offset.x + clip.x,
                    offset.y + clip.y,
                    clip.x, clip.y,
                    clip.w, clip.h,
                    Gdiplus::UnitPixel
                );
            }
        }
        
        ReleaseDC(hwnd, hdc);
    }
    
    void Validate() {
        is_dirty_ = false;
        dirty_rect_ = {};
        clip_regions_.clear();
    }
    
    void Invalidate(const Rect& rect = {}) {
        is_dirty_ = true;
        
        if (rect.w > 0 && rect.h > 0) {
            if (dirty_rect_.w == 0 || dirty_rect_.h == 0) {
                dirty_rect_ = rect;
            } else {
                dirty_rect_ = dirty_rect_.Union(rect);
            }
            
            // Add to clip regions for partial update
            clip_regions_.push_back(rect);
        } else {
            // Invalidate entire canvas
            dirty_rect_ = Rect{
                0, 0,
                static_cast<uint32_t>(bound_.w),
                static_cast<uint32_t>(bound_.h)
            };
            clip_regions_.clear();
        }
    }
    
    bool IsValid() const {
        return bitmap_ && bitmap_->IsValid() && 
               bound_.w > 0 && bound_.h > 0;
    }
    
    bool IsInvalidated() const {
        return is_dirty_;
    }
    
    Size GetSize() const {
        return bound_.GetSize();
    }
    
    uint32_t GetWidth() const {
        return static_cast<uint32_t>(bound_.w);
    }
    
    uint32_t GetHeight() const {
        return static_cast<uint32_t>(bound_.h);
    }
    
    const RectF& GetBound() const {
        return bound_;
    }
    
    void SetBound(const RectF& bound) {
        if (bound_.w != bound.w || bound_.h != bound.h) {
            Resize(bound.GetSize());
        }
        bound_ = bound;
    }
    
    Gdiplus::Bitmap* GetFrontBuffer() const {
        return bitmap_ ? bitmap_->front.get() : nullptr;
    }
    
    Gdiplus::Bitmap* GetBackBuffer() const {
        return bitmap_ ? bitmap_->back.get() : nullptr;
    }
    
    void SwapBuffers() {
        if (bitmap_ && bitmap_->front && bitmap_->back) {
            std::swap(bitmap_->front, bitmap_->back);
            is_dirty_ = false;
        }
    }
    
    const Rect& GetDirtyRect() const {
        return dirty_rect_;
    }
    
    operator bool() const {
        return IsValid();
    }
};

} // namespace zketch