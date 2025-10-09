<<<<<<< HEAD
#pragma once
#include "window.hpp"

namespace zketch {

	class Renderer {
	private :
		std::unique_ptr<Gdiplus::Graphics> gfx_ {} ;
		Canvas* canvas_target_ = nullptr ;
		Window* window_target_ = nullptr ;
		bool is_drawing_ = false ;

		bool IsValid() const noexcept {
			if (!canvas_target_) {

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
		gfx_(std::move(o.gfx_)), canvas_target_(std::exchange(o.canvas_target_, nullptr)), 
		is_drawing_(std::exchange(o.is_drawing_, false)) {}

		Renderer& operator=(Renderer&& o) noexcept {
			if (this != &o) {
				if (is_drawing_) {
					End() ;
				} 

				gfx_ = std::move(o.gfx_) ;
				canvas_target_ = std::exchange(o.canvas_target_, nullptr) ;
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

			auto* bmp = src.GetBitmap() ;
			if (!bmp) {
				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - source bitmap is null!") ;
				#endif

				return false ;
			}

			gfx_ = std::make_unique<Gdiplus::Graphics>(bmp) ;
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

			canvas_target_ = &src ;
			is_drawing_ = true ;

			gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
			gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
			gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
			gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;

			return true ;
		}

		bool Begin(Window& window) noexcept {
			if (is_drawing_) {

				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - Already in drawing state!") ;
				#endif

				return false ;
			}

			if (!window.IsCanvasValid()) {

				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - Invalid canvas!") ;
				#endif

				return false ;
			}

			auto* bmp = window.back_buffer_->GetBitmap() ;
			if (!bmp) {
				#ifdef RENDERER_DEBUG
					logger::error("Renderer::Begin - source bitmap is null!") ;
				#endif

				return false ;
			}

			gfx_ = std::make_unique<Gdiplus::Graphics>(bmp) ;
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

			canvas_target_ = window.back_buffer_.get() ;
			window_target_ = &window ;
			is_drawing_ = true ;

			gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
			gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
			gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
			gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;

			return true ;
		}

		void End() noexcept {
			if (window_target_) {
				if (canvas_target_ && is_drawing_) {
					if (window_target_->front_buffer_ && window_target_->back_buffer_) {
						std::swap(window_target_->front_buffer_, window_target_->back_buffer_) ;
						canvas_target_->MarkValidate() ;
					}
				}
			}
			gfx_.reset() ;
			canvas_target_ = nullptr ;
			window_target_ = nullptr ;
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
			
			if (canvas_target_) {
				canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawRectangle(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillRect(const Rect& rect, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;

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

			canvas_target_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawEllipse(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillEllipse(const RectF& rect, const zketch::Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;
			Gdiplus::SolidBrush brush(color) ;
			Gdiplus::Font used_font = font ;
			gfx_->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit) ;
			Gdiplus::RectF layout(static_cast<Gdiplus::REAL>(pos.x), static_cast<Gdiplus::REAL>(pos.y), static_cast<Gdiplus::REAL>(canvas_target_ ? canvas_target_->GetWidth() - pos.x : 0), static_cast<Gdiplus::REAL>(canvas_target_ ? canvas_target_->GetHeight() - pos.y : 0));
			Gdiplus::StringFormat fmt ;
			fmt.SetAlignment(Gdiplus::StringAlignmentNear) ;
			fmt.SetLineAlignment(Gdiplus::StringAlignmentNear) ;
			gfx_->DrawString(text.c_str(), -1, &used_font, layout, &fmt, &brush) ;
		}

		void DrawString(const std::string& text, const Point& pos, const Color& color, const Font& font) noexcept {
			DrawString(StringToWideString(text), pos, color, font) ;
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

			canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;
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

			canvas_target_->MarkInvalidate() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawLine(&p, start.x, start.y, end.x, end.y) ;
		}

		void DrawCircle(const Point& center, float radius, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			canvas_target_->MarkInvalidate() ;
			DrawEllipse(RectF{static_cast<float>(center.x - radius), static_cast<float>(center.y - radius), radius * 2.0f, radius * 2.0f}, color, thickness) ;
		}

		void FillCircle(const Point& center, float radius, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			canvas_target_->MarkInvalidate() ;
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
			canvas_target_->MarkInvalidate() ;
		}

		bool IsDrawing() const noexcept { return is_drawing_ ; }
		Canvas* GetTarget() const noexcept { return canvas_target_ ; }
	} ;
=======
#pragma once

#include "canvas.hpp"

namespace zketch {

	class Drawer {
	private:
		std::unique_ptr<Gdiplus::Graphics> gfx_ = nullptr ;
		Canvas* to_ = nullptr ;
		bool is_drawing_ = false ;

		bool IsValid() const noexcept {
			if (!to_) {
				logger::warning("Renderer - target canvas is null!") ;
				return false ;
			}

			if (!gfx_ || !is_drawing_) {
				if (!gfx_) {
					logger::warning("Renderer - gfx is nullptr!") ;
				}
				if (!is_drawing_) {
					logger::warning("Renderer - not in drawing state!") ;
				}
				return false ;
			}
			return true ;
		}

	public:
		Drawer() = default ;
		~Drawer() noexcept {
			if (is_drawing_) {
				logger::warning("Renderer destroyed while drawing - calling End().") ;
				End() ;
			}
		}

		Drawer(const Drawer&) = delete ;
		Drawer& operator=(const Drawer&) = delete ;

		Drawer(Drawer&& o) noexcept
			: gfx_(std::move(o.gfx_)), to_(std::exchange(o.to_, nullptr))
			, is_drawing_(std::exchange(o.is_drawing_, false)) {}

		Drawer& operator=(Drawer&& o) noexcept {
			if (this != &o) {
				if (is_drawing_) End() ;
				gfx_ = std::move(o.gfx_) ;
				to_ = std::exchange(o.to_, nullptr) ;
				is_drawing_ = std::exchange(o.is_drawing_, false) ;
			}
			return *this ;
		}

		bool Begin(Canvas& src, bool auto_clear = true) noexcept {
			if (is_drawing_) {
				logger::error("Renderer Begin() already in drawing state.") ;
				return false ;
			}

			if (!src.IsValid()) {
				logger::error("Renderer Begin() invalid canvas.") ;
				return false ;
			}

			auto* back = src.GetBackBuffer() ;
			if (!back) {
				logger::error("Renderer Begin() - back buffer is null.") ;
				return false ;
			}

			gfx_ = std::make_unique<Gdiplus::Graphics>(back) ;
			if (!gfx_) {
				logger::error("Renderer Begin() failed to create graphics object.") ;
				return false ;
			}

			if (gfx_->GetLastStatus() != Gdiplus::Ok) {
				logger::error("Renderer Begin() graphics status not OK: ", static_cast<int32_t>(gfx_ ? gfx_->GetLastStatus() : Gdiplus::GenericError)) ;
				gfx_.reset() ;
				return false ;
			}

			to_ = &src ;
			is_drawing_ = true ;

			// Quality settings BEFORE any drawing
			gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
			gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
			gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
			gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;

			// FIX: Use SourceCopy mode for clearing to completely replace pixels
			// This prevents artifacts when using transparent backgrounds
			if (auto_clear) {
				gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy) ;
				gfx_->Clear(static_cast<Gdiplus::Color>(src.GetClearColor())) ;
			}
			
			// Set to SourceOver for normal drawing operations
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;

			return true ;
		}

		void End() noexcept {
			if (to_ && is_drawing_) {
				to_->SwapBuffers() ;
			}

			gfx_.reset() ;
			to_ = nullptr ;
			is_drawing_ = false ;
		}

		void Clear(const Color& color) noexcept {
			if (!IsValid()) 
				return ;
			
			// FIX: Use SourceCopy to completely replace pixels (not blend)
			// This ensures transparent clear actually clears everything
			auto prevMode = gfx_->GetCompositingMode() ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceCopy) ;
			gfx_->Clear(color) ;
			gfx_->SetCompositingMode(prevMode) ;
			
			if (to_) to_->MarkDirty() ;
		}

		void DrawRect(const RectF& rect, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.1f) {
				logger::warning("DrawRect - thickness lower than 0.1") ;
				return ;
			}

			to_->MarkDirty() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawRectangle(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillRect(const Rect& rect, const Color& color) noexcept {
			if (!IsValid()) 
				return ;

			to_->MarkDirty() ;
			Gdiplus::SolidBrush b(color) ;
			gfx_->FillRectangle(&b, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void DrawRectRounded(const RectF& rect, const Color& color, float radius, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (thickness < 0.1f || radius < 0.0f) 
				return ;

			to_->MarkDirty() ;
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
				return ;
			}

			to_->MarkDirty() ;

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

			if (thickness < 0.1f) {
				return ;
			}

			to_->MarkDirty() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawEllipse(&p, static_cast<Gdiplus::RectF>(rect)) ;
		}

		void FillEllipse(const RectF& rect, const zketch::Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			to_->MarkDirty() ;
			Gdiplus::SolidBrush b(color) ;
			gfx_->FillEllipse(&b, rect.x, rect.y, rect.w, rect.h) ;
		}

		void DrawString(const std::wstring& text, const Point& pos, const Color& color, const Font& font) noexcept {
			if (!IsValid()) {
				return ;
			}

			if (text.empty()) {
				return ;
			}

			to_->MarkDirty() ;
			Gdiplus::SolidBrush brush(color) ;
			Gdiplus::FontFamily family(font.GetFamily().c_str()) ;
			Gdiplus::Font f(&family, font.GetSize(), font.GetStyle(), Gdiplus::UnitPixel) ;

			gfx_->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit) ;

			Gdiplus::RectF layout(static_cast<Gdiplus::REAL>(pos.x), static_cast<Gdiplus::REAL>(pos.y), static_cast<Gdiplus::REAL>(to_ ? to_->GetWidth() - pos.x : 0), static_cast<Gdiplus::REAL>(to_ ? to_->GetHeight() - pos.y : 0));

			Gdiplus::StringFormat fmt ;
			fmt.SetAlignment(Gdiplus::StringAlignmentNear) ;
			fmt.SetLineAlignment(Gdiplus::StringAlignmentNear) ;

			gfx_->DrawString(text.c_str(), -1, &f, layout, &fmt, &brush) ;
		}

		void DrawPolygon(const Vertex& vertices, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid() || vertices.empty()) {
				return ;
			}

			if (thickness < 0.1f) {
				return ;
			}

			to_->MarkDirty() ;
			std::vector<Gdiplus::PointF> points ;
			points.reserve(vertices.size()) ;

			for (const auto& v : vertices) {
				points.emplace_back(v.x, v.y) ;
			}

			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawPolygon(&p, points.data(), static_cast<int>(points.size())) ;
		}

		void FillPolygon(const Vertex& vertices, const Color& color) noexcept {
			if (!IsValid() || vertices.empty()) {
				return ;
			}

			to_->MarkDirty() ;
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

			if (thickness < 0.1f) {
				return ;
			}

			to_->MarkDirty() ;
			Gdiplus::Pen p(color, thickness) ;
			gfx_->DrawLine(&p, start.x, start.y, end.x, end.y) ;
		}

		void DrawCircle(const Point& center, float radius, const Color& color, float thickness = 1.0f) noexcept {
			if (!IsValid()) {
				return ;
			}

			to_->MarkDirty() ;
			DrawEllipse(RectF{static_cast<float>(center.x - radius), static_cast<float>(center.y - radius), radius * 2.0f, radius * 2.0f}, color, thickness) ;
		}

		void FillCircle(const Point& center, float radius, const Color& color) noexcept {
			if (!IsValid()) {
				return ;
			}

			to_->MarkDirty() ;
			FillEllipse(RectF{static_cast<float>(center.x - radius), static_cast<float>(center.y - radius), radius * 2.0f, radius * 2.0f}, color) ;
		}

		bool IsDrawing() const noexcept { 
			return is_drawing_ ; 
		}

		Canvas* GetTarget() const noexcept { 
			return to_ ; 
		}

		// Helper to composite another canvas onto current target
		void DrawCanvas(const Canvas* src, float x, float y) noexcept {
			if (!IsValid() || !src || !src->IsValid()) return;
			auto* bitmap = src->GetBitmap();
			if (bitmap) {
				gfx_->DrawImage(bitmap, x, y);
				if (to_) to_->MarkDirty();
			}
		}
	} ;
>>>>>>> dc9e717570a8202f64dc92753ab1b4f737c2a5c6
}