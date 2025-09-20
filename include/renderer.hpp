#pragma once

#include "font.hpp"

namespace zketch {

	class Drawer ; // forward

	class Canvas {
		friend class Drawer ;

	private:
		std::unique_ptr<Gdiplus::Bitmap> front_{} ;
		std::unique_ptr<Gdiplus::Bitmap> back_{} ;
		Point size_{} ;
		bool dirty_ = false ;

	public:
		Canvas(const Canvas&) = delete ;
		Canvas& operator=(const Canvas&) = delete ;
		Canvas() = default ;
		Canvas(Canvas&&) = default ;
		Canvas& operator=(Canvas&&) = default ;
		~Canvas() = default ;

		bool Create(const Point& size) noexcept {
			Clear() ;

			if (size.x <= 0 || size.y <= 0) {
				logger::warning("Canvas size is invalid.") ;
				return false ;
			}

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

			{
				Gdiplus::Graphics gfx_front(front_.get()) ;
				Gdiplus::Graphics gfx_back(back_.get()) ;
				gfx_front.Clear(Gdiplus::Color(255, 0, 0, 0)) ;
				gfx_back.Clear(Gdiplus::Color(255, 0, 0, 0)) ;
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

		Point GetSize() const noexcept { 
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

		bool Begin(Canvas& src) noexcept {
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

			// quality settings
			gfx_->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality) ;
			gfx_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic) ;
			gfx_->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality) ;
			gfx_->SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			gfx_->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;

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
			gfx_->Clear(color) ;
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
			Gdiplus::FontFamily family(font.family().c_str()) ;
			Gdiplus::Font f(&family, font.size(), font.style(), Gdiplus::UnitPixel) ;

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
	} ;
}