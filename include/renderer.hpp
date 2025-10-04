#pragma once
#include "canvas.hpp"

namespace zketch {

class Renderer {
private:
    std::unique_ptr<Gdiplus::Graphics> gfx_;
    Canvas* target_ = nullptr;
    bool is_drawing_ = false;
    Color clear_color_ = rgba(0, 0, 0, 0);
    bool partial_update_ = false;
    Rect update_region_;

public:
    Renderer() noexcept = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    
    Renderer(Renderer&& o) noexcept
        : gfx_(std::move(o.gfx_))
        , target_(std::exchange(o.target_, nullptr))
        , is_drawing_(std::exchange(o.is_drawing_, false))
        , clear_color_(o.clear_color_)
        , partial_update_(o.partial_update_)
        , update_region_(o.update_region_)
    {}
    
    Renderer& operator=(Renderer&& o) noexcept {
        if (this != &o) {
            gfx_ = std::move(o.gfx_);
            target_ = std::exchange(o.target_, nullptr);
            is_drawing_ = std::exchange(o.is_drawing_, false);
            clear_color_ = o.clear_color_;
            partial_update_ = o.partial_update_;
            update_region_ = o.update_region_;
        }
        return *this;
    }
    
    bool Begin(Canvas& canvas, bool full_redraw = true) {
        if (is_drawing_) {
            logger::error("Renderer::Begin - Already in drawing state");
            return false;
        }
        
        if (!canvas.IsValid()) {
            logger::error("Renderer::Begin - Invalid canvas");
            return false;
        }
        
        auto* back_buffer = canvas.GetBackBuffer();
        if (!back_buffer) {
            logger::error("Renderer::Begin - Back buffer is null");
            return false;
        }
        
        try {
            gfx_ = std::make_unique<Gdiplus::Graphics>(back_buffer);
        } catch (...) {
            logger::error("Renderer::Begin - Failed to create Graphics");
            return false;
        }
        
        if (!gfx_ || gfx_->GetLastStatus() != Gdiplus::Ok) {
            logger::error("Renderer::Begin - Graphics status not OK");
            gfx_.reset();
            return false;
        }
        
        target_ = &canvas;
        is_drawing_ = true;
        clear_color_ = canvas.GetClearColor();
        
        // Setup rendering quality
        gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
        gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
        gfx_->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
        
        // Determine update strategy
        if (full_redraw || !canvas.IsInvalidated()) {
            // Full redraw
            partial_update_ = false;
            Clear(clear_color_);
        } else {
            // Partial update
            partial_update_ = true;
            update_region_ = canvas.GetDirtyRect();
            
            if (update_region_.w > 0 && update_region_.h > 0) {
                // Set clip region for optimization
                Gdiplus::Rect clip_rect(
                    update_region_.x,
                    update_region_.y,
                    update_region_.w,
                    update_region_.h
                );
                gfx_->SetClip(clip_rect);
                
                // Clear only dirty region
                auto prev_mode = gfx_->GetCompositingMode();
                gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
                
                Gdiplus::SolidBrush clear_brush(clear_color_);
                gfx_->FillRectangle(&clear_brush, clip_rect);
                
                gfx_->SetCompositingMode(prev_mode);
            }
        }
        
        return true;
    }
    
    void End() {
        if (target_ && is_drawing_) {
            // Reset clip
            if (partial_update_) {
                gfx_->ResetClip();
            }
            
            target_->SwapBuffers();
            target_->Validate();
        }
        
        gfx_.reset();
        target_ = nullptr;
        is_drawing_ = false;
        partial_update_ = false;
        update_region_ = {};
    }
    
    bool IsValid() const {
        return target_ && gfx_ && is_drawing_;
    }
    
    bool IsDrawing() const {
        return is_drawing_;
    }
    
    Canvas* GetTarget() const {
        return target_;
    }
    
    bool IsPartialUpdate() const {
        return partial_update_;
    }
    
    const Rect& GetUpdateRegion() const {
        return update_region_;
    }
    
    void Clear(const Color& color) {
        if (!IsValid()) return;
        
        auto prev_mode = gfx_->GetCompositingMode();
        gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
        gfx_->Clear(color);
        gfx_->SetCompositingMode(prev_mode);
        
        if (target_) {
            target_->Invalidate();
        }
    }
    
    // Optimized drawing methods with dirty tracking
    
    void DrawRect(const RectF& rect, const Color& color, float thickness = 1.0f) {
        if (!IsValid() || thickness < 0.1f) return;
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawRectangle(&pen, static_cast<Gdiplus::RectF>(rect));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x - thickness),
                static_cast<int32_t>(rect.y - thickness),
                static_cast<uint32_t>(rect.w + thickness * 2),
                static_cast<uint32_t>(rect.h + thickness * 2)
            });
        }
    }

	void FillRect(const RectF& rect, const Color& color) {
        if (!IsValid()) return;
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillRectangle(&brush, static_cast<Gdiplus::RectF>(rect));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }

	void DrawRectRounded(const RectF& rect, const Color& color, float radius, float thickness) {
        if (!IsValid() || radius < 0.0f || thickness < 0.0f) return;
        
        Gdiplus::GraphicsPath path;
        float diameter = radius * 2.0f;
        
        path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90);
        path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90);
        path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90);
        path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90);
        path.CloseFigure();
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawPath(&pen, &path);
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x - thickness),
                static_cast<int32_t>(rect.y - thickness),
                static_cast<uint32_t>(rect.w + thickness * 2),
                static_cast<uint32_t>(rect.h + thickness * 2)
            });
        }
    }
    
    void FillRectRounded(const RectF& rect, const Color& color, float radius) {
        if (!IsValid() || radius < 0.0f) return;
        
        Gdiplus::GraphicsPath path;
        float diameter = radius * 2.0f;
        
        path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90);
        path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90);
        path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90);
        path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90);
        path.CloseFigure();
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillPath(&brush, &path);
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }
    
    void DrawEllipse(const RectF& rect, const Color& color, float thickness = 1.0f) {
        if (!IsValid() || thickness < 0.1f) return;
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawEllipse(&pen, static_cast<Gdiplus::RectF>(rect));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x - thickness),
                static_cast<int32_t>(rect.y - thickness),
                static_cast<uint32_t>(rect.w + thickness * 2),
                static_cast<uint32_t>(rect.h + thickness * 2)
            });
        }
    }
    
    void FillEllipse(const RectF& rect, const Color& color) {
        if (!IsValid()) return;
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillEllipse(&brush, rect.x, rect.y, rect.w, rect.h);
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }
    
    void DrawString(const std::wstring& text, const Point& pos, const Color& color, const Font& font) {
        if (!IsValid() || text.empty()) return;
        
        Gdiplus::SolidBrush brush(color);
        Gdiplus::FontFamily family(font.GetFamily().c_str());
        Gdiplus::Font gdi_font(&family, font.GetSize(), font.GetStyle(), Gdiplus::UnitPixel);
        
        Gdiplus::RectF layout(
            static_cast<Gdiplus::REAL>(pos.x),
            static_cast<Gdiplus::REAL>(pos.y),
            static_cast<Gdiplus::REAL>(target_ ? target_->GetWidth() - pos.x : 0),
            static_cast<Gdiplus::REAL>(target_ ? target_->GetHeight() - pos.y : 0)
        );
        
        Gdiplus::StringFormat fmt;
        fmt.SetAlignment(Gdiplus::StringAlignmentNear);
        fmt.SetLineAlignment(Gdiplus::StringAlignmentNear);
        
        gfx_->DrawString(text.c_str(), -1, &gdi_font, layout, &fmt, &brush);
        
        // Estimate text bounds for invalidation
        if (target_) {
            Gdiplus::RectF bounds;
            gfx_->MeasureString(text.c_str(), -1, &gdi_font, layout, &fmt, &bounds);
            
            target_->Invalidate(Rect{
                static_cast<int32_t>(bounds.X),
                static_cast<int32_t>(bounds.Y),
                static_cast<uint32_t>(bounds.Width + 2),
                static_cast<uint32_t>(bounds.Height + 2)
            });
        }
    }
    
    void DrawPolygon(const Vertex& vertices, const Color& color, float thickness = 1.0f) {
        if (!IsValid() || vertices.empty() || thickness < 0.1f) return;
        
        std::vector<Gdiplus::PointF> points;
        points.reserve(vertices.size());
        
        float min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max();
        float max_x = -std::numeric_limits<float>::max(), max_y = -std::numeric_limits<float>::max();
        
        for (const auto& v : vertices) {
            points.emplace_back(v.x, v.y);
            min_x = std::min(min_x, v.x);
            min_y = std::min(min_y, v.y);
            max_x = std::max(max_x, v.x);
            max_y = std::max(max_y, v.y);
        }
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawPolygon(&pen, points.data(), static_cast<int>(points.size()));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x - thickness),
                static_cast<int32_t>(min_y - thickness),
                static_cast<uint32_t>(max_x - min_x + thickness * 2),
                static_cast<uint32_t>(max_y - min_y + thickness * 2)
            });
        }
    }
    
    void FillPolygon(const Vertex& vertices, const Color& color) {
        if (!IsValid() || vertices.empty()) return;
        
        std::vector<Gdiplus::PointF> points;
        points.reserve(vertices.size());
        
        float min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max();
        float max_x = -std::numeric_limits<float>::max(), max_y = -std::numeric_limits<float>::max();
        
        for (const auto& v : vertices) {
            points.emplace_back(v.x, v.y);
            min_x = std::min(min_x, v.x);
            min_y = std::min(min_y, v.y);
            max_x = std::max(max_x, v.x);
            max_y = std::max(max_y, v.y);
        }
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillPolygon(&brush, points.data(), static_cast<int>(points.size()));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x),
                static_cast<int32_t>(min_y),
                static_cast<uint32_t>(max_x - min_x),
                static_cast<uint32_t>(max_y - min_y)
            });
        }
    }
    
    void DrawLine(const Point& start, const Point& end, const Color& color, float thickness = 1.0f) {
        if (!IsValid() || thickness < 0.1f) return;
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawLine(&pen, start.x, start.y, end.x, end.y);
        
        if (target_) {
            int32_t min_x = std::min(start.x, end.x);
            int32_t min_y = std::min(start.y, end.y);
            int32_t max_x = std::max(start.x, end.x);
            int32_t max_y = std::max(start.y, end.y);
            
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x - thickness),
                static_cast<int32_t>(min_y - thickness),
                static_cast<uint32_t>(max_x - min_x + thickness * 2),
                static_cast<uint32_t>(max_y - min_y + thickness * 2)
            });
        }
    }
    
    void DrawCircle(const Point& center, float radius, const Color& color, float thickness = 1.0f) {
        if (!IsValid()) return;
        
        DrawEllipse(
            RectF{
                static_cast<float>(center.x - radius),
                static_cast<float>(center.y - radius),
                radius * 2.0f,
                radius * 2.0f
            },
            color,
            thickness
        );
    }
    
    void FillCircle(const Point& center, float radius, const Color& color) {
        if (!IsValid()) return;
        
        FillEllipse(
            RectF{
                static_cast<float>(center.x - radius),
                static_cast<float>(center.y - radius),
                radius * 2.0f,
                radius * 2.0f
            },
            color
        );
    }
    
    void DrawPath(const Vertex& vertices, const Color& color, float thickness = 1.0f, bool closed = false) {
        if (!IsValid() || vertices.empty() || thickness < 0.1f) return;
        
        Gdiplus::GraphicsPath path;
        
        if (vertices.size() >= 2) {
            path.StartFigure();
            
            for (size_t i = 0; i < vertices.size() - 1; ++i) {
                path.AddLine(vertices[i].x, vertices[i].y, 
                           vertices[i+1].x, vertices[i+1].y);
            }
            
            if (closed && vertices.size() >= 3) {
                path.CloseFigure();
            }
        }
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawPath(&pen, &path);
        
        if (target_) {
            // Calculate bounding box
            float min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max();
            float max_x = -std::numeric_limits<float>::max(), max_y = -std::numeric_limits<float>::max();
            
            for (const auto& v : vertices) {
                min_x = std::min(min_x, v.x);
                min_y = std::min(min_y, v.y);
                max_x = std::max(max_x, v.x);
                max_y = std::max(max_y, v.y);
            }
            
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x - thickness),
                static_cast<int32_t>(min_y - thickness),
                static_cast<uint32_t>(max_x - min_x + thickness * 2),
                static_cast<uint32_t>(max_y - min_y + thickness * 2)
            });
        }
    }
    
    void FillPath(const Vertex& vertices, const Color& color) {
        if (!IsValid() || vertices.empty()) return;
        
        Gdiplus::GraphicsPath path;
        
        if (vertices.size() >= 3) {
            path.StartFigure();
            
            for (size_t i = 0; i < vertices.size() - 1; ++i) {
                path.AddLine(vertices[i].x, vertices[i].y, 
                           vertices[i+1].x, vertices[i+1].y);
            }
            
            path.CloseFigure();
        }
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillPath(&brush, &path);
        
        if (target_) {
            float min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max();
            float max_x = -std::numeric_limits<float>::max(), max_y = -std::numeric_limits<float>::max();
            
            for (const auto& v : vertices) {
                min_x = std::min(min_x, v.x);
                min_y = std::min(min_y, v.y);
                max_x = std::max(max_x, v.x);
                max_y = std::max(max_y, v.y);
            }
            
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x),
                static_cast<int32_t>(min_y),
                static_cast<uint32_t>(max_x - min_x),
                static_cast<uint32_t>(max_y - min_y)
            });
        }
    }
    
    void DrawBezier(const PointF& p1, const PointF& p2, const PointF& p3, const PointF& p4,
                    const Color& color, float thickness = 1.0f) {
        if (!IsValid() || thickness < 0.1f) return;
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawBezier(&pen, 
            Gdiplus::PointF(p1.x, p1.y),
            Gdiplus::PointF(p2.x, p2.y),
            Gdiplus::PointF(p3.x, p3.y),
            Gdiplus::PointF(p4.x, p4.y)
        );
        
        if (target_) {
            // Bounding box for bezier curve
            float min_x = std::min({p1.x, p2.x, p3.x, p4.x});
            float min_y = std::min({p1.y, p2.y, p3.y, p4.y});
            float max_x = std::max({p1.x, p2.x, p3.x, p4.x});
            float max_y = std::max({p1.y, p2.y, p3.y, p4.y});
            
            target_->Invalidate(Rect{
                static_cast<int32_t>(min_x - thickness),
                static_cast<int32_t>(min_y - thickness),
                static_cast<uint32_t>(max_x - min_x + thickness * 2),
                static_cast<uint32_t>(max_y - min_y + thickness * 2)
            });
        }
    }
    
    void DrawArc(const RectF& rect, float start_angle, float sweep_angle,
                 const Color& color, float thickness = 1.0f) {
        if (!IsValid() || thickness < 0.1f) return;
        
        Gdiplus::Pen pen(color, thickness);
        gfx_->DrawArc(&pen, 
            static_cast<Gdiplus::RectF>(rect),
            start_angle, sweep_angle
        );
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x - thickness),
                static_cast<int32_t>(rect.y - thickness),
                static_cast<uint32_t>(rect.w + thickness * 2),
                static_cast<uint32_t>(rect.h + thickness * 2)
            });
        }
    }
    
    void FillPie(const RectF& rect, float start_angle, float sweep_angle, const Color& color) {
        if (!IsValid()) return;
        
        Gdiplus::SolidBrush brush(color);
        gfx_->FillPie(&brush,
            static_cast<Gdiplus::RectF>(rect),
            start_angle, sweep_angle
        );
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }
    
    // Gradient fills
    
    void FillRectGradient(const RectF& rect, const Color& color1, const Color& color2, 
                         bool horizontal = true) {
        if (!IsValid()) return;
        
        Gdiplus::LinearGradientBrush brush(
            static_cast<Gdiplus::RectF>(rect),
            color1,
            color2,
            horizontal ? Gdiplus::LinearGradientModeHorizontal : Gdiplus::LinearGradientModeVertical
        );
        
        gfx_->FillRectangle(&brush, static_cast<Gdiplus::RectF>(rect));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }
    
    void FillEllipseGradient(const RectF& rect, const Color& color1, const Color& color2) {
        if (!IsValid()) return;
        
        Gdiplus::GraphicsPath path;
        path.AddEllipse(static_cast<Gdiplus::RectF>(rect));
        
        Gdiplus::PathGradientBrush brush(&path);
        brush.SetCenterColor(color1);
        
        Gdiplus::Color colors[] = {color2};
        int count = 1;
        brush.SetSurroundColors(colors, &count);
        
        gfx_->FillEllipse(&brush, static_cast<Gdiplus::RectF>(rect));
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(rect.x),
                static_cast<int32_t>(rect.y),
                static_cast<uint32_t>(rect.w),
                static_cast<uint32_t>(rect.h)
            });
        }
    }
    
    // Image drawing
    
    void DrawImage(Gdiplus::Bitmap* image, const RectF& dest_rect) {
        if (!IsValid() || !image) return;
        
        gfx_->DrawImage(image,
            static_cast<Gdiplus::RectF>(dest_rect)
        );
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(dest_rect.x),
                static_cast<int32_t>(dest_rect.y),
                static_cast<uint32_t>(dest_rect.w),
                static_cast<uint32_t>(dest_rect.h)
            });
        }
    }
    
    void DrawImage(Gdiplus::Bitmap* image, const RectF& dest_rect, const RectF& src_rect) {
        if (!IsValid() || !image) return;
        
        gfx_->DrawImage(image,
            static_cast<Gdiplus::RectF>(dest_rect),
            src_rect.x, src_rect.y, src_rect.w, src_rect.h,
            Gdiplus::UnitPixel
        );
        
        if (target_) {
            target_->Invalidate(Rect{
                static_cast<int32_t>(dest_rect.x),
                static_cast<int32_t>(dest_rect.y),
                static_cast<uint32_t>(dest_rect.w),
                static_cast<uint32_t>(dest_rect.h)
            });
        }
    }
    
    // Utility methods
    
    void SetClipRect(const RectF& rect) {
        if (!IsValid()) return;
        
        Gdiplus::Rect clip_rect(
            static_cast<int>(rect.x),
            static_cast<int>(rect.y),
            static_cast<int>(rect.w),
            static_cast<int>(rect.h)
        );
        
        gfx_->SetClip(clip_rect);
    }
    
    void ResetClip() {
        if (!IsValid()) return;
        gfx_->ResetClip();
    }
    
    void SetOpacity(float alpha) {
        if (!IsValid()) return;
        
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        
        Gdiplus::ColorMatrix matrix = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, alpha, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        
        // Note: This requires ImageAttributes for full implementation
        // For now, just store the value
    }
    
    void SaveState() {
        if (!IsValid()) return;
        gfx_->Save();
    }
    
    void RestoreState() {
        if (!IsValid()) return;
        gfx_->Restore(Gdiplus::GraphicsState());
    }
    
    void Translate(float dx, float dy) {
        if (!IsValid()) return;
        gfx_->TranslateTransform(dx, dy);
    }
    
    void Rotate(float angle) {
        if (!IsValid()) return;
        gfx_->RotateTransform(angle);
    }
    
    void Scale(float sx, float sy) {
        if (!IsValid()) return;
        gfx_->ScaleTransform(sx, sy);
    }
    
    void ResetTransform() {
        if (!IsValid()) return;
        gfx_->ResetTransform();
    }
    
    // Query methods
    
    RectF MeasureString(const std::wstring& text, const Font& font) {
        if (!IsValid() || text.empty()) {
            return RectF{0, 0, 0, 0};
        }
        
        Gdiplus::FontFamily family(font.GetFamily().c_str());
        Gdiplus::Font gdi_font(&family, font.GetSize(), font.GetStyle(), Gdiplus::UnitPixel);
        
        Gdiplus::RectF layout(0, 0, 10000, 10000);
        Gdiplus::RectF bounds;
        
        Gdiplus::StringFormat fmt;
        fmt.SetAlignment(Gdiplus::StringAlignmentNear);
        fmt.SetLineAlignment(Gdiplus::StringAlignmentNear);
        
        gfx_->MeasureString(text.c_str(), -1, &gdi_font, layout, &fmt, &bounds);
        
        return RectF{bounds.X, bounds.Y, bounds.Width, bounds.Height};
    }
    
    Size GetCanvasSize() const {
        if (!target_) {
            return Size{0, 0};
        }
        return target_->GetSize();
    }
};

// Alias for backward compatibility
using Drawer = Renderer;

} // namespace zketch