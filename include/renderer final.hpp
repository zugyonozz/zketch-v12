#pragma once
#include "canvas_final.hpp"

namespace zketch {

	class Renderer {
	private :
		std::unique_ptr<Gdiplus::Graphics> gfx_ {} ;
		Canvas* src_ = nullptr ;
		bool is_drawing_ = false ;

		bool IsValid() const noexcept {
			if (!src_) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::IsValid - Target canvas is null!") ;
				#endif

				return false ;
			}

			if (!gfx_ || !is_drawing_) {
				if (!gfx_) {

					#ifdef RENDERER_DEBUG
						logger::warning("Renderer::IsValid - gfx is null!") ;
					#endif

				}

				if (!is_drawing_) {

					#ifdef RENDERER_DEBUG
						logger::warning("Renderer::IsValid - Not in drawing state!") ;
					#endif
				}

				return false ;
			}

			return true ;
		}

	public :
		Renderer(const Renderer&) = delete ;
		Renderer& operator=(const Renderer&) = delete ;
		Renderer() = default ;

		Renderer(Renderer&& o) noexcept : 
		gfx_(std::move(o.gfx_)), src_(std::exchange(o.src_, nullptr)), 
		is_drawing_(std::exchange(o.is_drawing_, false)) {}

		Renderer& operator=(Renderer&& o) noexcept {
			if (this != &o) {
				if (is_drawing_) {
					End() ;
				} 

				gfx_ = std::move(o.gfx_) ;
				src_ = std::exchange(o.src_, nullptr) ;
				is_drawing_ = std::exchange(o.is_drawing_, false) ;
			}

			return *this ;
		}

		~Renderer() noexcept {
			if (is_drawing_) {
				
				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::~Renderer - Destroyed while drawing, calling End().") ;
				#endif

				End() ;
			}
		}

		bool Begin(Canvas& src) noexcept {
			if (is_drawing_) {

				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - Already in drawing state!") ;
				#endif

				return false ;
			}

			if (!src.IsValid()) {

				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - Invalid canvas!") ;
				#endif

				return false ;
			}

			auto* back = src.GetBackBuffer() ;
			if (!back) {
				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - back buffer is null!") ;
				#endif

				return false ;
			}

			gfx_ = std::make_unique<Gdiplus::Graphics>(back) ;
			if (!gfx_) {
				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - Failed to create graphics object!") ;
				#endif

				return false ;
			}

			if (gfx_->GetLastStatus() != Gdiplus::Ok) {
				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - graphics status not OK : [", static_cast<int32_t>(gfx_ ? gfx_->GetLastStatus() : Gdiplus::GenericError), "] .") ;
				#endif

				gfx_.reset() ;
				return false ;
			}

			src_ = &src ;
			is_drawing_ = true ;

			gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
			gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
			gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
			gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;

			return true ;
		}

		void End() noexcept {
			if (src_ && is_drawing_) {
				src_->SwapBuffers() ;
			}

			gfx_.reset() ;
			src_ = nullptr ;
			is_drawing_ = false ;
		}

		void Clear(const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}
			
			auto prevMode = gfx_->GetCompositingMode() ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy) ;
			gfx_->Clear(color) ;
			gfx_->SetCompositingMode(prevMode) ;
			
			if (src_) {
				src_->MarkInvalidate() ;
			}
		}

		void DrawRect(const RectF& rect, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawRect - Thickness lower than 0.0") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawRectangle(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillRect(const Rect& rect, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::SolidBrush b(color) ;
			gfx_->FillRectangle(&b, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void DrawRectRounded(const RectF& rect, const Color& color, float radius, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawRectRounded - Thickness lower than 0.0") ;
				#endif

				return ;
			}

			if (radius < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawRectRounded - Radius lower than 0.0") ;
				#endif
				
				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::GraphicsPath path ;
			float diameter = radius * 2.0f ;
			path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90) ;
			path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90) ;
			path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90) ;
			path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90) ;
			path.CloseFigure() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawPath(&p, &path) ;
		}

		void FillRectRounded(const RectF& rect, const Color& color, float radius) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (radius < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::FillRectRounded - Radius lower than 0.0") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;

			Gdiplus::GraphicsPath path ;
			float diameter = radius * 2.0f ;
			path.AddArc(rect.x, rect.y, diameter, diameter, 180, 90) ;
			path.AddArc(rect.x + rect.w - diameter, rect.y, diameter, diameter, 270, 90) ;
			path.AddArc(rect.x + rect.w - diameter, rect.y + rect.h - diameter, diameter, diameter, 0, 90) ;
			path.AddArc(rect.x, rect.y + rect.h - diameter, diameter, diameter, 90, 90) ;
			path.CloseFigure() ;
			Gdiplus::SolidBrush b(color) ;
			gfx_->FillPath(&b, &path) ;
		}

		void DrawEllipse(const RectF& rect, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawEllipse - Thickness lower than 0.0") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawEllipse(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillEllipse(const RectF& rect, const zketch::Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::SolidBrush b(color) ;
			gfx_->FillEllipse(&b, rect.x, rect.y, rect.w, rect.h) ;
		}

		void DrawString(const std::wstring& text, const Point& pos, const Color& color, const Font& font) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (text.empty()) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawString - Text is empty") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::SolidBrush brush(color) ;
			Gdiplus::FontFamily family(font.GetFamily().c_str()) ;
			Gdiplus::Font f(&family, font.GetSize(), font.GetStyle(), Gdiplus::UnitPixel) ;
			gfx_->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit) ;
			Gdiplus::RectF layout(static_cast<Gdiplus::REAL>(pos.x), static_cast<Gdiplus::REAL>(pos.y), static_cast<Gdiplus::REAL>(src_ ? src_->GetWidth() - pos.x : 0), static_cast<Gdiplus::REAL>(src_ ? src_->GetHeight() - pos.y : 0));
			Gdiplus::StringFormat fmt ;
			fmt.SetAlignment(Gdiplus::StringAlignmentNear) ;
			fmt.SetLineAlignment(Gdiplus::StringAlignmentNear) ;
			gfx_->DrawString(text.c_str(), -1, &f, layout, &fmt, &brush) ;
		}

		void DrawPolygon(const Vertex& vertices, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (vertices.empty()) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawPolygon - Vertices is Empty") ;
				#endif

				return ;
			}

			if (thickness < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawPolygon - Thickness lower than 0.0") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			std::vector<Gdiplus::PointF> points ;
			points.reserve(vertices.size()) ;

			for (const auto& v : vertices) {
				points.emplace_back(v.x, v.y) ;
			}

			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawPolygon(&p, points.data(), static_cast<int>(points.size())) ;
		}

		void FillPolygon(const Vertex& vertices, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (vertices.empty()) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::FillPolygon - Vertices is Empty") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			std::vector<Gdiplus::PointF> points ;
			points.reserve(vertices.size()) ;
			
			for (const auto& v : vertices) {
				points.emplace_back(v.x, v.y) ;
			}

			Gdiplus::SolidBrush b(color) ;
			gfx_->FillPolygon(&b, points.data(), static_cast<int>(points.size())) ;
		}

		void DrawLine(const Point& start, const Point& end, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.0f) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawLine - Thickness lower than 0.0") ;
				#endif

				return ;
			}

			src_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawLine(&p, start.x, start.y, end.x, end.y) ;
		}

		void DrawCircle(const Point& center, float radius, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			src_->MarkInvalidate() ;
			DrawEllipse(RectF{static_cast<float>(center.x - radius), static_cast<float>(center.y - radius), radius * 2.0f, radius * 2.0f}, color, thickness) ;
		}

		void FillCircle(const Point& center, float radius, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			src_->MarkInvalidate() ;
			FillEllipse(RectF{static_cast<float>(center.x - radius), static_cast<float>(center.y - radius), radius * 2.0f, radius * 2.0f}, color) ;
		}

		void DrawCanvas(const Canvas* src, const Point& pos) noexcept {
			if (!IsValid() || !src->IsValid()) { 
				return ; 
			}

			if (!src) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawCanvas - Canvas source is null!") ;
				#endif

				return ;
			}

			auto* bitmap = src->GetBitmap() ;
			if (!bitmap) {

				#ifdef RENDERER_DEBUG
					logger::warning("Renderer::DrawCanvas - Bitmap is null!") ;
				#endif

				return ;
			}

			gfx_->DrawImage(bitmap, pos.x, pos.y ) ;
			src_->MarkInvalidate() ;
		}

		void Present(Canvas* canvas, HWND handle) const noexcept {
			if (!canvas || !canvas->IsValid()) {
				logger::warning("Window::Display - invalid canvas") ;
				return ;
			}

			HDC hdc = GetDC(handle) ;
			if (!hdc) {
				logger::warning("Canvas::Present - invalid HDC.") ;
				return ;
			}

			Gdiplus::Graphics screen(hdc) ;
			screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			screen.DrawImage(canvas->GetBitmap(), 0, 0) ;

			ReleaseDC(handle, hdc) ;
		}

		bool IsDrawing() const noexcept { return is_drawing_ ; }
		Canvas* GetTarget() const noexcept { return src_ ; }
	} ;
}