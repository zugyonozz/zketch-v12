#pragma once
#include "canvas.hpp"

namespace zketch {

    // Forward declaration untuk Layer system
    struct DrawCommand {
        std::function<void()> callback ;
        uint64_t id = 0 ;
        int32_t z_order = 0 ; // Layer ordering

        bool operator<(const DrawCommand& other) const noexcept {
            return z_order < other.z_order ;
        }
    } ;

    class Renderer {
    private:
        std::unique_ptr<Gdiplus::Graphics> gfx_ ;
        Canvas* target_ = nullptr ;
        bool is_drawing_ = false ;
        Color clear_color_ = rgba(0, 0, 0, 0) ;

    public:
        Renderer() noexcept = default ;
        Renderer(const Renderer&) = delete ;
        Renderer& operator=(const Renderer&) = delete ;

        Renderer(Renderer&& o) noexcept : 
            gfx_(std::move(o.gfx_)), 
            target_(std::exchange(o.target_, nullptr)), 
            is_drawing_(std::exchange(o.is_drawing_, false)),
            clear_color_(o.clear_color_) {}

        Renderer& operator=(Renderer&& o) noexcept {
            if (this != &o) {
                gfx_ = std::move(o.gfx_) ;
                target_ = std::exchange(o.target_, nullptr) ;
                is_drawing_ = std::exchange(o.is_drawing_, false) ;
                clear_color_ = o.clear_color_ ;
            }
            return *this ;
        }

        bool Begin(Canvas& canvas) noexcept {
            if (is_drawing_) {
                logger::error("Renderer::Begin - Already in drawing state") ;
                return false ;
            }

            if (!canvas.IsValid()) {
                logger::error("Renderer::Begin - Invalid canvas") ;
                return false ;
            }

            auto* back_buffer = canvas.GetBackBuffer() ;
            if (!back_buffer) {
                logger::error("Renderer::Begin - Back buffer is null") ;
                return false ;
            }

            try {
                gfx_ = std::make_unique<Gdiplus::Graphics>(back_buffer) ;
            } catch (...) {
                logger::error("Renderer::Begin - Failed to create Graphics") ;
                return false ;
            }

            if (!gfx_ || gfx_->GetLastStatus() != Gdiplus::Ok) {
                logger::error("Renderer::Begin - Graphics status not OK") ;
                gfx_.reset() ;
                return false ;
            }

            target_ = &canvas ;
            is_drawing_ = true ;
            clear_color_ = canvas.GetClearColor() ;

            // Setup rendering quality
            gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
            gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
            gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
            gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
            gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
            gfx_->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit) ;

            // Auto-clear dengan clear color
            Clear(clear_color_) ;

            return true ;
        }

        void End() noexcept {
            if (target_ && is_drawing_) {
                target_->SwapBuffers() ;
            }

            gfx_.reset() ;
            target_ = nullptr ;
            is_drawing_ = false ;
        }

        bool IsValid() const noexcept {
            return target_ && gfx_ && is_drawing_ ;
        }

        bool IsDrawing() const noexcept { 
            return is_drawing_ ; 
        }

        Canvas* GetTarget() const noexcept { 
            return target_ ; 
        }

        void Clear(const Color& color) noexcept {
            if (!IsValid()) return ;
            
            auto prev_mode = gfx_->GetCompositingMode() ;
            gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy) ;
            gfx_->Clear(color) ;
            gfx_->SetCompositingMode(prev_mode) ;
            
            if (target_) {
                target_->Invalidate() ;
            }
        }

        void DrawRect(const RectF& rect, const Color& color, float thickness = 1.0f) noexcept {
            if (!IsValid() || thickness < 0.1f) return ;
            
            target_->Invalidate() ;
            Gdiplus::Pen pen(color, thickness) ;
            gfx_->DrawRectangle(&pen, static_cast<Gdiplus::RectF>(rect)) ;
        }

        void FillRect(const RectF& rect, const Color& color) noexcept {
            if (!IsValid()) return ;
            
            target_->Invalidate() ;
            Gdiplus::SolidBrush brush(color) ;
            gfx_->FillRectangle(&brush, static_cast<Gdiplus::RectF>(rect)) ;
        }

        void DrawRectRounded(const RectF& rect, const Color& color, float radius, float thickness = 1.0f) noexcept {
            if (!IsValid() || thickness < 0.1f || radius < 0.0f) return ;

            target_->Invalidate() ;
            Gdiplus::GraphicsPath path ;
            float diameter = radius * 2.0f ;
            
            path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90) ;
            path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90) ;
            path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90) ;
            path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90) ;
            path.CloseFigure() ;

            Gdiplus::Pen pen(color, thickness) ;
            gfx_->DrawPath(&pen, &path) ;
        }

        void FillRectRounded(const RectF& rect, const Color& color, float radius) noexcept {
            if (!IsValid() || radius < 0.0f) return ;

            target_->Invalidate() ;
            Gdiplus::GraphicsPath path ;
            float diameter = radius * 2.0f ;
            
            path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90) ;
            path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90) ;
            path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90) ;
            path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90) ;
            path.CloseFigure() ;

            Gdiplus::SolidBrush brush(color) ;
            gfx_->FillPath(&brush, &path) ;
        }

        void DrawEllipse(const RectF& rect, const Color& color, float thickness = 1.0f) noexcept {
            if (!IsValid() || thickness < 0.1f) return ;
            
            target_->Invalidate() ;
            Gdiplus::Pen pen(color, thickness) ;
            gfx_->DrawEllipse(&pen, static_cast<Gdiplus::RectF>(rect)) ;
        }

        void FillEllipse(const RectF& rect, const Color& color) noexcept {
            if (!IsValid()) return ;
            
            target_->Invalidate() ;
            Gdiplus::SolidBrush brush(color) ;
            gfx_->FillEllipse(&brush, rect.x, rect.y, rect.w, rect.h) ;
        }

        void DrawString(const std::wstring& text, const Point& pos, const Color& color, const Font& font) noexcept {
            if (!IsValid() || text.empty()) return ;

            target_->Invalidate() ;
            Gdiplus::SolidBrush brush(color) ;
            Gdiplus::FontFamily family(font.GetFamily().c_str()) ;
            Gdiplus::Font gdi_font(&family, font.GetSize(), font.GetStyle(), Gdiplus::UnitPixel) ;

            Gdiplus::RectF layout(
                static_cast<Gdiplus::REAL>(pos.x), 
                static_cast<Gdiplus::REAL>(pos.y), 
                static_cast<Gdiplus::REAL>(target_ ? target_->GetWidth() - pos.x : 0), 
                static_cast<Gdiplus::REAL>(target_ ? target_->GetHeight() - pos.y : 0)
            ) ;

            Gdiplus::StringFormat fmt ;
            fmt.SetAlignment(Gdiplus::StringAlignmentNear) ;
            fmt.SetLineAlignment(Gdiplus::StringAlignmentNear) ;

            gfx_->DrawString(text.c_str(), -1, &gdi_font, layout, &fmt, &brush) ;
        }

        void DrawPolygon(const Vertex& vertices, const Color& color, float thickness = 1.0f) noexcept {
            if (!IsValid() || vertices.empty() || thickness < 0.1f) return ;

            target_->Invalidate() ;
            std::vector<Gdiplus::PointF> points ;
            points.reserve(vertices.size()) ;

            for (const auto& v : vertices) {
                points.emplace_back(v.x, v.y) ;
            }

            Gdiplus::Pen pen(color, thickness) ;
            gfx_->DrawPolygon(&pen, points.data(), static_cast<int>(points.size())) ;
        }

        void FillPolygon(const Vertex& vertices, const Color& color) noexcept {
            if (!IsValid() || vertices.empty()) return ;

            target_->Invalidate() ;
            std::vector<Gdiplus::PointF> points ;
            points.reserve(vertices.size()) ;
            
            for (const auto& v : vertices) {
                points.emplace_back(v.x, v.y) ;
            }

            Gdiplus::SolidBrush brush(color) ;
            gfx_->FillPolygon(&brush, points.data(), static_cast<int>(points.size())) ;
        }

        void DrawLine(const Point& start, const Point& end, const Color& color, float thickness = 1.0f) noexcept {
            if (!IsValid() || thickness < 0.1f) return ;

            target_->Invalidate() ;
            Gdiplus::Pen pen(color, thickness) ;
            gfx_->DrawLine(&pen, start.x, start.y, end.x, end.y) ;
        }

        void DrawCircle(const Point& center, float radius, const Color& color, float thickness = 1.0f) noexcept {
            if (!IsValid()) return ;
            
            DrawEllipse(
                RectF{
                    static_cast<float>(center.x - radius), 
                    static_cast<float>(center.y - radius), 
                    radius * 2.0f, 
                    radius * 2.0f
                }, 
                color, 
                thickness
            ) ;
        }

        void FillCircle(const Point& center, float radius, const Color& color) noexcept {
            if (!IsValid()) return ;
            
            FillEllipse(
                RectF{
                    static_cast<float>(center.x - radius), 
                    static_cast<float>(center.y - radius), 
                    radius * 2.0f, 
                    radius * 2.0f
                }, 
                color
            ) ;
        }
    } ;

    // Alias untuk backward compatibility
    using Drawer = Renderer ;
}