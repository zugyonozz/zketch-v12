#pragma once
#include "widget.hpp"

namespace zketch {
	
	class InputBox : public Widget<InputBox> {
		friend class Widget<InputBox> ;

	private :
		uint16_t cursor_interval_ = 500 ;
		uint32_t cursor_timer_ = 0 ;
		bool cursor_visible_ = false ;
		bool is_active_ = false ;
		bool is_hovered_ = false ;
		std::chrono::steady_clock::time_point last_update_ ;
		size_t cursor_index_ = 0 ;
		PointF text_offset_ = {} ;
		std::wstring text_ ;
		Font font_ ;
		std::function<void(Canvas*, const InputBox&)> drawing_logic_ ;
		std::function<void()> callback_ ;

		void UpdateImpl() noexcept {
			if (!drawing_logic_) {
				return ;
			}

			if (!IsValid()) {
				return ;
			}

			drawing_logic_(canvas_.get(), *this) ;
		}

		void AutoScrollToCursor() noexcept {
			if (text_.empty() || cursor_index_ == 0) {
				text_offset_.x = 0 ;
				return ;
			}

			std::wstring before_cursor = text_.substr(0, cursor_index_) ;
			float cursor_x = font_.GetStringWidth(before_cursor) ;
			float visible_width = bound_.w - 20.0f ; 

			if (cursor_x + text_offset_.x > visible_width) {
				text_offset_.x = visible_width - cursor_x ;
			}

			if (cursor_x + text_offset_.x < 0) {
				text_offset_.x = -cursor_x ;
			}

			if (text_offset_.x > 0) {
				text_offset_.x = 0 ;
			}
		}

	public :
		InputBox(const RectF& bound, const Font& font, uint32_t cursor_interval_ms = 500) noexcept {
			canvas_ = std::make_unique<Canvas>() ;

			bound_ = bound ;
			cursor_interval_ = cursor_interval_ms ;
			cursor_index_ = 0 ;
			font_ = font ;
			canvas_->Create(bound_.GetSize()) ;

			SetDrawingLogic([](Canvas* canvas, const InputBox& input){
				Renderer render ;

				Color bg_color ;
				Color border_color ;
				
				if (input.IsActive()) {
					bg_color = rgba(255, 255, 255, 1) ;
					border_color = rgba(100, 149, 237, 1) ; // Blue when active
				} else if (input.IsHovered()) {
					bg_color = rgba(240, 240, 240, 1) ;
					border_color = rgba(150, 150, 150, 1) ;
				} else {
					bg_color = rgba(230, 230, 230, 1) ;
					border_color = rgba(180, 180, 180, 1) ;
				}

				if (!render.Begin(*canvas)) {
					return ;
				}

				render.Clear(Transparent) ;

				render.FillRectRounded(
					input.GetRelativeBound(), 
					bg_color, 
					5.0f
				) ;

				render.DrawRectRounded(
					input.GetRelativeBound(), 
					border_color, 
					5.0f, 
					2.0f
				) ;

				if (!input.GetText().empty()) {
					Point text_pos = {
						static_cast<int32_t>(input.GetTextOffset().x + 10), 
						static_cast<int32_t>(input.GetRelativeBound().h / 2 - input.GetFont().GetHeight() / 2)
					} ;

					render.DrawString(
						input.GetText(), 
						text_pos, 
						rgba(40, 40, 40, 1), 
						input.GetFont()
					) ;
				} else if (input.GetText().empty() && !input.IsActive()) {
					Point placeholder_pos = {
						10, 
						static_cast<int32_t>(input.GetRelativeBound().h / 2 - input.GetFont().GetHeight() / 2)
					} ;

					render.DrawString(
						L"test inputtext", 
						placeholder_pos, 
						rgba(150, 150, 150, 1), // Gray color for placeholder
						input.GetFont()
					) ;
				}
                
                if (input.IsActive() && input.IsCursorVisible()) {
					float cursor_x = input.GetTextOffset().x + 10 ;
					
					if (input.GetCursorIndex() > 0 && input.GetCursorIndex() <= input.GetText().size()) {
						std::wstring before_cursor = input.GetText().substr(0, input.GetCursorIndex()) ;
						cursor_x += input.GetFont().GetStringWidth(before_cursor) ;
					}
					
					float cursor_y = input.GetRelativeBound().h / 2 - input.GetFont().GetHeight() / 2 ;
					float cursor_height = input.GetFont().GetHeight() ;
					
					render.DrawLine(
						{static_cast<int32_t>(cursor_x), static_cast<int32_t>(cursor_y)},
						{static_cast<int32_t>(cursor_x), static_cast<int32_t>(cursor_y + cursor_height)},
						rgba(40, 40, 40, 1),
						2.0f
					) ;
				}

                render.End() ;
			}) ;

			last_update_ = std::chrono::steady_clock::now() ;
		}

		// common method

		bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                update_ = true ;
            }
            return state ;
		}

		bool OnPress(const PointF& mouse_pos) noexcept {
			bool was_active = is_active_ ;
			
			if (bound_.Contain(mouse_pos)) {
				// Activate and position cursor
				is_active_ = true ;
				
				float click_x = mouse_pos.x - bound_.x - text_offset_.x - 10 ;
				
				if (click_x <= 0) {
					cursor_index_ = 0 ;
				} else {
					float accumulated_width = 0 ;
					cursor_index_ = text_.size() ;
					
					for (size_t i = 0; i < text_.size(); ++i) {
						wchar_t wc = text_[i] ;
						// Handle wide char by converting to narrow for width calculation
						char nc = (wc < 128) ? static_cast<char>(wc) : '?' ;
						float char_width = font_.GetCharWidthOf(nc) ;
						
						if (accumulated_width + char_width / 2 > click_x) {
							cursor_index_ = i ;
							break ;
						}
						accumulated_width += char_width ;
					}
				}
				
				// Reset cursor blink
				cursor_visible_ = true ;
				cursor_timer_ = 0 ;
				update_ = true ;
				
				return true ;
			} else {
				// Clicked outside - deactivate
				if (was_active) {
					is_active_ = false ;
					cursor_visible_ = false ;
					update_ = true ;
				}
				return false ;
			}
		}

		bool OnRelease(const PointF& mouse_pos) noexcept {
			return !bound_.Contain(mouse_pos) ;
		}

		void UpdateCursor() noexcept {
			if (!is_active_) {
				cursor_visible_ = false ;
				return ;
			}

			auto now = std::chrono::steady_clock::now() ;
			auto delta_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_).count() ;
			cursor_timer_ += delta_time_ms ;
			if (cursor_timer_ >= cursor_interval_) {
				cursor_visible_ = !cursor_visible_ ;
				cursor_timer_ = 0 ;
				update_ = true ;
			}
			last_update_ = now ;
		}

		void MoveCursorNext() noexcept { 
			if (cursor_index_ < text_.size()) {
				++cursor_index_ ;
				AutoScrollToCursor() ;
				cursor_visible_ = true ;
				cursor_timer_ = 0 ;
				update_ = true ;
			}
		}

		void MoveCursorPrev() noexcept { 
			if (cursor_index_ > 0) {
				--cursor_index_ ;
				AutoScrollToCursor() ;
				cursor_visible_ = true ;
				cursor_timer_ = 0 ;
				update_ = true ;
			}
		}

		void MoveCursorToStart() noexcept {
			cursor_index_ = 0 ;
			AutoScrollToCursor() ;
			cursor_visible_ = true ;
			cursor_timer_ = 0 ;
			update_ = true ;
		}

		void MoveCursorToEnd() noexcept {
			cursor_index_ = text_.size() ;
			AutoScrollToCursor() ;
			cursor_visible_ = true ;
			cursor_timer_ = 0 ;
			update_ = true ;
		}

		void Insert(wchar_t c) noexcept { 
			text_.insert(text_.begin() + cursor_index_, c) ;
			++cursor_index_ ;
			AutoScrollToCursor() ;
			cursor_visible_ = true ;
			cursor_timer_ = 0 ;
			update_ = true ;
		}

		void Backspace() noexcept { 
			if (cursor_index_ > 0) {
				text_.erase(cursor_index_ - 1, 1) ;
				--cursor_index_ ;
				AutoScrollToCursor() ;
				cursor_visible_ = true ;
				cursor_timer_ = 0 ;
				update_ = true ;
			}
		}

		void Delete() noexcept {
			if (cursor_index_ < text_.size()) {
				text_.erase(cursor_index_, 1) ;
				AutoScrollToCursor() ;
				cursor_visible_ = true ;
				cursor_timer_ = 0 ;
				update_ = true ;
			}
		}

		void Clear() noexcept {
			text_.clear() ;
			cursor_index_ = 0 ;
			text_offset_.x = 0 ;
			cursor_visible_ = true ;
			cursor_timer_ = 0 ;
			update_ = true ;
		}

		void Submit() noexcept {
			is_active_ = false ;
			cursor_visible_ = false ;
			update_ = true ;
			
			if (callback_) {
				callback_() ;
			}
		}

		void Deactivate() noexcept {
			cursor_visible_ = false ;
			update_ = true ;
		}

		// setter

		void SetText(const std::wstring_view& text) noexcept {
			text_ = text ;
			cursor_index_ = text_.size() ;
			AutoScrollToCursor() ;
			update_ = true ;
		}

		void SetCursorInterval(uint32_t ms) noexcept { 
			cursor_interval_ = ms ; 
			update_ = true ;
		}

		void SetCursorIndex(size_t index) noexcept { 
			cursor_index_ = index ; 
			AutoScrollToCursor() ;
			update_ = true ;
		}

		void SetTextOffset(const PointF& offset) noexcept { 
			text_offset_ = offset ; 
			update_ = true ;
		}

		void SetDrawingLogic(std::function<void(Canvas*, const InputBox&)> drawing_logic) noexcept {
			drawing_logic_ = std::move(drawing_logic) ;
			update_ = true ;
		}

		void SetCallback(std::function<void()> callback) noexcept {
			callback_ = std::move(callback) ;
		}

		// getter

		const Font& GetFont() const noexcept { return font_ ; }
		const size_t GetCursorIndex() const noexcept { return cursor_index_ ; }
		const PointF& GetTextOffset() const noexcept { return text_offset_ ; }
		const std::wstring& GetText() const noexcept { return text_ ; }
		RectF GetRelativeBound() const noexcept { return {PointF{}, bound_.GetSize()} ; }

		// state query

		bool IsCursorVisible() const noexcept { return cursor_visible_ ; }
		bool IsActive() const noexcept { return is_active_ ; }
		bool IsHovered() const noexcept { return is_hovered_ ; }
		bool IsEmpty() const noexcept { return text_.empty() ; }
	} ;
}