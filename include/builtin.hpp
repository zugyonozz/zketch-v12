#pragma once
#include "window.hpp"

namespace zketch {
	class Slider {
	public :
		enum Orientation : uint8_t {
			Vertical,
			Horizontal
		} ;

	private :
		Orientation orientation_ ;
		RectF track_bound_ ;
		RectF thumb_bound_ ;
		float value_ ;
		float offset_ = 0 ;
		bool is_dragging_ ;
		std::unique_ptr<Canvas> canvas_ ;
		std::function<void(Canvas*)> drawer_ ;

		float GetMinValue() const noexcept {
			return 0.0f ;
		}

		float GetMaxValue() const noexcept {
			return orientation_ == Vertical ? track_bound_.h - thumb_bound_.h : track_bound_.w - thumb_bound_.w ;
		}

	public :
		Slider(Orientation orientation, const RectF& track, const SizeF& thumb) noexcept : orientation_(orientation), track_bound_(track), canvas_(std::make_unique<Canvas>()) {
			thumb_bound_ = (orientation_ == Vertical ? RectF{PointF{(track.x + track.w) / 2 - thumb.x / 2, track.y}, thumb} : RectF{PointF{track.x, (track.y + track.h) / 2 - thumb.y / 2}, thumb}) ;
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
				offset_ = orientation_ == Vertical ? e.GetMousePosition().y - thumb_bound_.y : e.GetMousePosition().x - thumb_bound_.x ;
			}

			return state ;
		}

		bool OnRelease(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Up) {
				return false ;
			}

			EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::End, value_)) ;
			is_dragging_ = false ;

			return true ;
		}

		bool OnDrag(const Event& e) noexcept {
			if (!is_dragging_) {
				return false ;
			}

			if (orientation_ == Horizontal) {
				float newX = e.GetMousePosition().x - offset_ ;
				newX = std::clamp(newX, track_bound_.x, track_bound_.x + track_bound_.w - thumb_bound_.w) ;
				thumb_bound_.x = newX ;

				float ratio = (newX - track_bound_.x) / (track_bound_.w - thumb_bound_.w) ;
				value_ = ratio * GetMaxValue() ;
			} else {
				float newY = e.GetMousePosition().y - offset_ ;
				newY = std::clamp(newY, track_bound_.y, track_bound_.y + track_bound_.h - thumb_bound_.h) ;
				thumb_bound_.y = newY ;

				float ratio = (newY - track_bound_.y) / (track_bound_.h - thumb_bound_.h) ;
				value_ = ratio * GetMaxValue() ;
			}

			EventSystem::PushEvent(Event::CreateSliderEvent(SliderState::Changed, value_)) ;

			return true ;
		}

		void Update() noexcept {
			if (!drawer_) {
				logger::warning("Slider::Update() - drawer is null") ;
				return ;
			}

			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Slider::Update() - Canvas not valid") ;
				return ;
			}

			drawer_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Slider::Present() - Canvas not valid") ;
				return ;
			}

			canvas_->Present(hwnd, {static_cast<int32_t>(track_bound_.x), static_cast<int32_t>(track_bound_.y)}) ;
		}

		void SetDrawer(std::function<void(Canvas*)> drawer) noexcept {
			drawer_ = std::move(drawer) ;
		}

		float GetValue() const noexcept { return value_ ; }
		const RectF& GetThumbBound() const noexcept { return thumb_bound_ ; }
		const RectF& GetBound() const noexcept { return track_bound_ ; }
		const Canvas* GetCanvas() const noexcept { return canvas_.get() ; }
	} ;

	class TextBox {
	private :
		std::unique_ptr<Font> font_ ;
		std::unique_ptr<std::wstring> text_ ;
		Anchor anchor_ ;
		RectF bound_ ;
		PointF text_pos_ ;
		std::unique_ptr<Canvas> canvas_ ;
		std::function<void(Canvas*)> update_ ;
	
	public :
		TextBox(const TextBox&) = delete ;
		TextBox& operator=(const TextBox&) = delete ;
		TextBox(TextBox&&) noexcept = default ;
		TextBox& operator=(TextBox&&) noexcept = default ;

		TextBox(const Font& font, const RectF& bound, Anchor anchor = Anchor::Left) noexcept : 
		anchor_(anchor), bound_(bound), text_pos_(bound.Anchor(anchor)), canvas_(std::make_unique<Canvas>()) {
			font_ = std::make_unique<Font>(font) ;
			text_ = std::make_unique<std::wstring>(L"") ;
			canvas_->Create(bound_.getSize()) ;
		}

		~TextBox() noexcept = default ;

		TextBox& operator+=(std::wstring& text) noexcept {
			*text_ += text ;
			return *this ;
		}

		std::wstring& operator--() noexcept {
			if (!text_->empty()) {
				text_->pop_back() ;
			}
			return *text_ ;
		}

		void SetTextPosition(const PointF& pos) noexcept {
			text_pos_ = (bound_.Anchor(anchor_) - GetTextBound().getSize()) + pos ;
		}

		void SetTextPosition(Anchor anchor, const PointF& pos) noexcept { // relative bound_
			anchor_ = anchor ;
			text_pos_ = (bound_.Anchor(anchor_) - GetTextBound().getSize()) + pos ;
		}

		void SetUpdate(std::function<void(Canvas*)> update) noexcept {
			update_ = std::move(update) ;
		}

		RectF GetTextBound() const noexcept {
			Gdiplus::Graphics g(canvas_->GetBitmap()) ;
			Gdiplus::RectF ret ;
			Gdiplus::Font f(font_->family().data(), font_->size(), font_->style()) ;

			g.MeasureString(
				text_->c_str(), text_->length(), 
				&f,
				text_pos_,
				&ret
			) ;

			return static_cast<RectF>(ret) ;
		}

		const Font& GetFont() const noexcept { return *font_ ; }
		const std::wstring& GetText() const noexcept { return *text_ ; }
		const PointF& GetTextPosition() const noexcept { return text_pos_ ; }
		const Anchor& GetAnchor() const noexcept { return anchor_ ; }
		const RectF& GetBound() const noexcept { return bound_ ; }
		const Canvas* GetCanvas() const noexcept { return canvas_.get() ; }

		void Update() noexcept {
			if (!update_) {
				logger::warning("TextBox::Update() - update function is null") ;
				return ;
			}

			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("TextBox::Update() - Canvas not valid") ;
				return ;
			}

			update_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("TextBox::Present() - Canvas not valid") ;
				return ;
			}

			canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
		}
	} ;

	class Button {
	private :
		RectF bound_ = {} ;
		std::unique_ptr<Canvas> canvas_ ;
		std::function<void(Canvas*)> drawer_ ;

	public :
		Button(RectF bound) noexcept : bound_(bound), canvas_(std::make_unique<Canvas>()) {
			canvas_->Create(bound_.getSize()) ;
		}

		~Button() noexcept = default ;

		void SetDrawer(std::function<void(Canvas*)> drawer) noexcept {
			drawer_ = std::move(drawer) ;
		}

		const RectF& GetBound() const noexcept { return bound_ ; }
		RectF& GetBound() noexcept { return bound_ ; }

		bool OnHover(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::None) {
				return false ;
			}

			bool state = bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Hover, this)) ;
			}

			return state ;
		}

		bool OnPress(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Down) {
				return false ;
			}

			bool state = bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Press, this)) ;
			}

			return state ;
		}

		bool OnRelease(const Event& e) noexcept {
			if (!e.IsMouseEvent() || e.GetMouseState() != MouseState::Up) {
				return false ;
			}

			bool state = bound_.Contain(e.GetMousePosition()) ;
			if (state) {
				EventSystem::PushEvent(Event::CreateButtonEvent(ButtonState::Release, this)) ;
			}
			return state ;
		}

		void Update() noexcept {
			if (!drawer_) {
				logger::warning("Button::Update() - drawer is null") ;
				return ;
			}

			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Button::Update() - Canvas not valid") ;
				return ;
			}

			drawer_(canvas_.get()) ;
		}

		void Present(HWND hwnd) noexcept {
			if (!canvas_ || !canvas_->IsValid()) {
				logger::warning("Button::Present() - Canvas not valid") ;
				return ;
			}

			canvas_->Present(hwnd, {static_cast<int32_t>(bound_.x), static_cast<int32_t>(bound_.y)}) ;
		}
	} ;
}