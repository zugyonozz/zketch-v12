#pragma once

#include <cstdint>
#include <utility>
#include <memory>
#include <unordered_map>

#include "win32init.h"
#include "gdiplusinit.h"

#include "logger.h"

namespace zketch {

	enum class FontStyle : uint8_t {
		Regular = 0,
		Bold = 1,
		Italic = 2,
		BoldItalic = 3,
		Underline = 4,
		Strikeout = 8
	} ;

	const char* FontStyleToString(FontStyle style_) noexcept {
		switch (style_) {
			case FontStyle::Regular : return "Regular" ;
			case FontStyle::Bold : return "Bold" ;
			case FontStyle::Italic : return "Italic" ;
			case FontStyle::BoldItalic : return "BoldItalic" ;
			case FontStyle::Underline : return "Underline" ;
			case FontStyle::Strikeout : return "Strikeout" ;
			default : break ;
		}
		return "Undefined Style" ;
	}

	class Font {
	private :
		FontStyle style_ ;
		float size_ ;
		std::wstring family_ ;
	public :
		constexpr Font() noexcept : style_(FontStyle::Regular), size_(0.0f), family_() {}

		constexpr Font(const std::wstring& family_, float size_, FontStyle style_ = FontStyle::Regular) noexcept : style_(style_), size_(size_), family_(family_) {}

		constexpr Font(const Font& o) noexcept : style_(o.style_), size_(o.size_), family_(o.family_) {}

		constexpr Font(Font&& o) noexcept : style_(std::exchange(o.style_, FontStyle::Regular)), size_(std::exchange(o.size_, 0.0f)), family_(std::move(o.family_)) {}

		constexpr void SetStyle(FontStyle style_) noexcept {
			this->style_ = style_ ;
		}

		constexpr void SetSize(float size_) noexcept {
			this->size_ = size_ ;
		}

		constexpr void SetFamily(const std::wstring& family_) noexcept {
			this->family_ = family_ ;
		}

		constexpr FontStyle GetStyle() const noexcept {
			return style_ ;
		}

		constexpr float GetSize() const noexcept {
			return size_ ;
		}

		constexpr const std::wstring& GetFamily() const noexcept {
			return family_ ;
		}
	} ;

	class FontManager {
	private :
		using Fonts = std::unordered_map<std::wstring, std::unordered_map<FontStyle, std::unique_ptr<Font>>> ;

		static inline Fonts g_fonts_ ;

	public :
		static inline void CreateFont(const std::wstring& family_, float size_, FontStyle style_) noexcept {
			auto it = g_fonts_.find(family_) ;
			if (it != g_fonts_.end()) {
				logger::warning("Family font has Registered!\nError: Family ", std::string(family_.begin(), family_.end()), "\n") ;
				return ;
			}
			if (it->second.find(style_) != it->second.end()) {
				logger::warning("Family FontStyle has Registered!\nError: FontStyle ", FontStyleToString(style_), "\n") ;
				return ;
			}
			auto font = std::make_unique<Font>(family_, size_, style_) ;
			g_fonts_[font->GetFamily()][font->GetStyle()] = std::move(font) ;
		}

		const auto& operator[](const std::wstring font_) const noexcept {
			return g_fonts_[font_] ;
		}
	} ;
}