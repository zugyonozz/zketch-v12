#pragma once
#include "window.hpp"

namespace zketch {
	
	// ============================================
	// Base Widget Class untuk menghindari duplikasi
	// ============================================
	template <typename Derived>
	class Widget {
	protected:
		std::unique_ptr<Canvas> canvas_ ;
		RectF bound_ ;
		
		bool ValidateCanvas(const char* context) const noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning(context, " - Canvas not valid") ;
				return false ;
			}
			return true ;
		}
		
	public:
		Widget() noexcept = default ;
		~Widget() noexcept = default ;
		
		const RectF& GetBound() const noexcept { return static_cast<const Derived*>(this)->bound_ ; }
		RectF& GetBound() noexcept { return static_cast<Derived*>(this)->bound_ ; }
		const Canvas* GetCanvas() const noexcept { return static_cast<const Derived*>(this)->canvas_.get() ; }
		
		void Update() noexcept { return static_cast<Derived*>(this)->Update() ; }
		void Present(HWND hwnd) noexcept { return static_cast<Derived*>(this)->Present(hwnd) ; }
	} ;

	// ============================================
	// Slider Class
	// ============================================
	class Slider : public Widget<Slider> {
	public:
		enum Orientation : uint8_t {
			Vertical,
			Horizontal
		} ;

	private:
		Orientation orientation_ ;
		RectF track_bound_ ;
		RectF thumb_bound_ ;
		float value_ = 0.0f ;  // Inisialisasi eksplisit
		float offset_ = 0.0f ;
		bool is_dragging_ = false ;  // Inisialisasi eksplisit
		std::function<void(Canvas*)> drawer_ ;
		
		// Helper untuk clamp thumb position
		void ClampThumbPosition() noexcept {
			if (orientation_ == Horizontal) {
				thumb_bound_.x = std::clamp(
					thumb_bound_.x, 
					track_bound_.x, 
					track_bound_.x + track_bound_.w - thumb_bound_.w
				) ;
			} else {
				thumb_bound_.y = std::clamp(
					thumb_bound_.y, 
					track_bound_.y, 
					track_bound_.y + track_bound_.h - thumb_bound_.h
				) ;
			}
		}

	public:
		Slider(Orientation orientation, const RectF& track, const SizeF& thumb) noexcept : orientation_(orientation), track_bound_(track) {
			
			// Inisialisasi posisi thumb
			if (orientation_ == Vertical) {
				thumb_bound_ = RectF{
					PointF{(track.x + track.w) / 2 - thumb.x / 2, track.y}, 
					thumb
				} ;
			} else {
				thumb_bound_ = RectF{
					PointF{track.x, (track.y + track.h) / 2 - thumb.y / 2}, 
					thumb
				} ;
			}
			
			canvas_ = std::make_unique<Canvas>()  ;
			bound_ = track_bound_ ;  // Set bound untuk Widget base class
			canvas_->Create(track.getSize()) ;
		}

		~Slider() noexcept = default ;

		bool OnHover(const Event& e) const noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::None) {
				return false ;
			}

			bool state = thumb_bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Hover, value_)) ;
			}

			return state ;
		}

		bool OnPress(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Down) {
				return false ;
			}

			bool state = thumb_bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Start, value_)) ;
				is_dragging_ = true ;
				
				// Hitung offset relatif terhadap thumb
				offset_ = orientation_ == Vertical 
					? e.GetMousePosition().y - thumb_bound_.y 
					: e.GetMousePosition().x - thumb_bound_.x ;
			}

			return state ;
		}

		bool OnRelease(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Up) {
				return false ;
			}

			// Hanya kirim event jika sedang dragging
			if (is_dragging_) {
				EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_)) ;
				is_dragging_ = false ;
				return true ;
			}

			return false ;
		}

		bool OnDrag(const Event& e) noexcept {
			if (!is_dragging_) {
				return false ;
			}

			// Perbaikan: perhitungan value yang benar
			if (orientation_ == Horizontal) {
				float newX = e.GetMousePosition().x - offset_ ;
				newX = std::clamp(
					newX, 
					track_bound_.x, 
					track_bound_.x + track_bound_.w - thumb_bound_.w
				) ;
				thumb_bound_.x = newX ;
				
				// Value adalah posisi relatif terhadap track
				value_ = newX - track_bound_.x ;
			} else {
				float newY = e.GetMousePosition().y - offset_ ;
				newY = std::clamp(
					newY, 
					track_bound_.y, 
					track_bound_.y + track_bound_.h - thumb_bound_.h
				) ;
				thumb_bound_.y = newY ;
				
				// Value adalah posisi relatif terhadap track
				value_ = newY - track_bound_.y ;
			}

			EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_)) ;
			return true ;
		}

		void Update() noexcept {
			if (!drawer_) {
				logger::warning("Slider::Update() - drawer is null") ;
				return ;
			}

			if (!ValidateCanvas("Slider::Update()")) {
				return ;
			}

			drawer_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!ValidateCanvas("Slider::Present()")) {
				return ;
			}

			canvas_->Present(
				hwnd, 
				{static_cast<int32_t>(track_bound_.x), static_cast<int32_t>(track_bound_.y)}
			) ;
		}

		void SetDrawer(std::function<void(Canvas*)> drawer) noexcept {
			drawer_ = std::move(drawer) ;
		}
		
		// Helper untuk set value dari luar
		void SetValue(float value) noexcept {
			value_ = std::clamp(value, 0.0f, GetMaxValue()) ;
			
			// Update thumb position berdasarkan value
			if (orientation_ == Horizontal) {
				thumb_bound_.x = track_bound_.x + value_ ;
			} else {
				thumb_bound_.y = track_bound_.y + value_ ;
			}
			
			ClampThumbPosition() ;
		}
		
		// Get normalized value (0.0 - 1.0)
		float GetNormalizedValue() const noexcept {
			float max = GetMaxValue() ;
			return max > 0.0f ? value_ / max : 0.0f ;
		}

		float GetValue() const noexcept { return value_ ; }
		const RectF& GetThumbBound() const noexcept { return thumb_bound_ ; }
		RectF& GetThumbBound() noexcept { return thumb_bound_ ; }
		const RectF& GetTrackBound() const noexcept { return track_bound_ ; }
		RectF& GetTrackBound() noexcept { return track_bound_ ; }
		float GetMinValue() const noexcept { return 0.0f ; }
		float GetMaxValue() const noexcept { return orientation_ == Vertical ? track_bound_.h - thumb_bound_.h : track_bound_.w - thumb_bound_.w ; }
	} ;

	// ============================================
	// TextBox Class
	// ============================================
	class TextBox : public Widget<TextBox> {
	private:
		std::unique_ptr<Font> font_ ;
		std::unique_ptr<std::wstring> text_ ;
		Anchor anchor_ ;
		PointF text_pos_ ;
		std::function<void(Canvas*)> update_ ;
		
		// Cache untuk text bounds agar tidak perlu recalculate terus
		mutable RectF cached_text_bound_ ;
		mutable bool text_bound_dirty_ = true ;

	public:
		TextBox(const TextBox&) = delete ;
		TextBox& operator=(const TextBox&) = delete ;
		TextBox(TextBox&&) noexcept = default ;
		TextBox& operator=(TextBox&&) noexcept = default ;

		TextBox(const Font& font, const RectF& bound, Anchor anchor = Anchor::Left) noexcept 
			: anchor_(anchor), text_pos_(bound.Anchor(anchor)) {
			
			font_ = std::make_unique<Font>(font) ;
			text_ = std::make_unique<std::wstring>(L"") ;
			canvas_ = std::make_unique<Canvas>()  ;
			bound_ = bound ;
			canvas_->Create(bound_.getSize()) ;
		}

		~TextBox() noexcept = default ;

		TextBox& operator+=(const std::wstring& text) noexcept {
			*text_ += text ;
			text_bound_dirty_ = true ;  // Mark cache as dirty
			return *this ;
		}

		std::wstring& operator--() noexcept {
			if (!text_->empty()) {
				text_->pop_back() ;
				text_bound_dirty_ = true ;  // Mark cache as dirty
			}
			return *text_ ;
		}
		
		// Set text dengan invalidate cache
		void SetText(const std::wstring& text) noexcept {
			*text_ = text ;
			text_bound_dirty_ = true ;
		}

		void SetTextPosition(const PointF& pos) noexcept {
			text_pos_ = (bound_.Anchor(anchor_) - GetTextBound().getSize()) + pos ;
		}

		void SetTextPosition(Anchor anchor, const PointF& pos) noexcept {
			anchor_ = anchor ;
			text_pos_ = (bound_.Anchor(anchor_) - GetTextBound().getSize()) + pos ;
		}

		void SetUpdate(std::function<void(Canvas*)> update) noexcept {
			update_ = std::move(update) ;
		}

		// Gunakan caching untuk menghindari pembuatan Graphics terus-menerus
		RectF GetTextBound() const noexcept {
			if (!text_bound_dirty_) {
				return cached_text_bound_ ;
			}
			
			if (!canvas_ || !canvas_->IsValid()) {
				return RectF{} ;
			}
			
			Gdiplus::Graphics g(canvas_->GetBitmap()) ;
			Gdiplus::RectF ret ;
			Gdiplus::Font f(font_->family().data(), font_->size(), font_->style()) ;

			g.MeasureString(
				text_->c_str(), 
				static_cast<int>(text_->length()), 
				&f,
				text_pos_,
				&ret
			) ;

			cached_text_bound_ = static_cast<RectF>(ret) ;
			text_bound_dirty_ = false ;
			
			return cached_text_bound_ ;
		}

		const Font& GetFont() const noexcept { return *font_ ; }
		const std::wstring& GetText() const noexcept { return *text_ ; }
		const PointF& GetTextPosition() const noexcept { return text_pos_ ; }
		const Anchor& GetAnchor() const noexcept { return anchor_ ; }

		void Update() noexcept {
			if (!update_) {
				logger::warning("TextBox::Update() - update function is null") ;
				return ;
			}

			if (!ValidateCanvas("TextBox::Update()")) {
				return ;
			}

			update_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!ValidateCanvas("TextBox::Present()")) {
				return ;
			}

			canvas_->Present(
				hwnd, 
				{static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}
			) ;
		}
	} ;

	// ============================================
	// Button Class
	// ============================================
	class Button : public Widget<Button> {
	private:
		std::function<void(Canvas*)> drawer_ ;
		bool is_hovered_ = false ;   // Track hover state
		bool is_pressed_ = false ;   // Track press state

	public:
		Button(const RectF& bound) noexcept {
			canvas_ = std::make_unique<Canvas>()  ;
			bound_ = bound ;
			canvas_->Create(bound_.getSize()) ;
		}

		~Button() noexcept = default ;

		void SetDrawer(std::function<void(Canvas*)> drawer) noexcept {
			drawer_ = std::move(drawer) ;
		}

		bool OnHover(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::None) {
				return false ;
			}

			bool state = bound_.Contain(e.GetMousePosition()) ;
			
			// Hanya kirim event jika state berubah
			if (state != is_hovered_) {
				is_hovered_ = state ;
				if (state) {
					EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Hover, this)) ;
				}
			}

			return state ;
		}

		bool OnPress(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Down) {
				return false ;
			}

			bool state = bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				is_pressed_ = true ;
				EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Press, this)) ;
			}

			return state ;
		}

		bool OnRelease(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Up) {
				return false ;
			}

			bool was_pressed = is_pressed_ ;
			is_pressed_ = false ;
			
			bool state = bound_.Contain(e.GetMousePosition()) ;
			
			// Hanya kirim Release event jika button memang di-press sebelumnya
			// dan mouse masih di dalam bounds (proper click)
			if (state && was_pressed) {
				EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Release, this)) ;
				return true ;
			}
			
			return false ;
		}

		void Update() noexcept {
			if (!drawer_) {
				logger::warning("Button::Update() - drawer is null") ;
				return ;
			}

			if (!ValidateCanvas("Button::Update()")) {
				return ;
			}

			drawer_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!ValidateCanvas("Button::Present()")) {
				return ;
			}

			canvas_->Present(
				hwnd, 
				{static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}
			) ;
		}
		
		// Getters untuk state
		bool IsHovered() const noexcept { return is_hovered_ ; }
		bool IsPressed() const noexcept { return is_pressed_ ; }
	} ;
}