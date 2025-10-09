#pragma once
#include "unit.hpp"

namespace ___FONT_DATA___ {
	class __font_data__ ;
	class __font_binary_header__ ;
}

namespace ___FONT_DUMP___ {
	class __font_dump__ ;
}

namespace ___ERROR_HANDLER___ {
	struct no_have_access : public std::exception {
		const char* what() const noexcept override {
			return "Unauthorized access to font data structure. Only one declaration allowed." ;
		}
	} ;

	struct invalid_type : public std::exception {
		const char* what() const noexcept override {
			return "Invalid font type or font not found in the font database." ;
		}
	} ;
}

namespace ___ACCESSOR___ {
	class ____accessor____ {
		friend class ___FONT_DATA___::__font_data__ ;
		friend class ___FONT_DATA___::__font_binary_header__ ;
		friend class ___FONT_DUMP___::__font_dump__ ;

	private :
		static inline int ___grant_depth___ = 0 ;

		static inline void ___open_access___() noexcept {
			++___grant_depth___ ;
		}

		static inline void ___close_access___() noexcept {
			if (___grant_depth___ > 0) {
				--___grant_depth___ ;
			}
		}

		static inline bool ___is_granted___() noexcept {
			return ___grant_depth___ > 0 ;
		}
	} ;
}

namespace ___FONT_DATA___ {

	static constexpr uint8_t ___CHAR_START___ = 32 ;   // Space
	static constexpr uint8_t ___CHAR_END___ = 126 ;    // Tilde
	static constexpr uint8_t ___CHAR_COUNT___ = ___CHAR_END___ - ___CHAR_START___ + 1 ;  // 95 chars
	static constexpr uint8_t ___FONT_NAME_MAX_LEN___ = 192 ;

	#pragma pack(push, 1)

	class __font_data__ {
		friend class zketch::Font ;
		friend class zketch::Application ;
		friend class ___FONT_DUMP___::__font_dump__ ;

	private :
		uint8_t style_ = 0 ;
		float height_ = 0.0f ;
		float weight_ = 0.0f ;
		float ascent_ = 0.0f ;
		float descent_ = 0.0f ;
		std::array<char, ___FONT_NAME_MAX_LEN___> fontname_ {} ;
		std::array<float, ___CHAR_COUNT___> char_widths_ {} ;

		inline void SetFontName(const std::string_view& name) noexcept {
			const size_t len = std::min(name.size(), size_t(___FONT_NAME_MAX_LEN___ - 1)) ;
			memcpy(fontname_.data(), name.data(), len) ;
			fontname_[len] = '\0' ;
			if (len + 1 < ___FONT_NAME_MAX_LEN___) {
				memset(fontname_.data() + len + 1, 0, ___FONT_NAME_MAX_LEN___ - len - 1) ;
			}
		}

	public :
		__font_data__() {
			if (!___ACCESSOR___::____accessor____::___is_granted___()) {
				throw ___ERROR_HANDLER___::no_have_access() ;
			}
		}
	} ;

	#pragma pack(pop)

	#pragma pack(push, 1)
	class __font_binary_header__ {
		friend class ___FONT_DUMP___::__font_dump__ ;

	private :
		uint32_t magic_ = 0x534E5446 ;  // "FTNS" dalam little endian
		uint16_t version_ = 3 ;
		uint16_t entry_size_ = sizeof(__font_data__) ;
		uint32_t count_ = 0 ;
		uint64_t timestamp_ = 0 ;
		uint64_t reserved_1_ = 0 ;
		uint64_t reserved_2_ = 0 ;
		
	public :
		__font_binary_header__() {
			if (!___ACCESSOR___::____accessor____::___is_granted___()) {
				throw ___ERROR_HANDLER___::no_have_access() ;
			} 
		}
		
		[[nodiscard]] bool IsValid() const noexcept {
			return magic_ == 0x534E5446 && version_ == 3 &&  entry_size_ == sizeof(__font_data__) && count_ > 0 ; 
		}
	} ;
	#pragma pack(pop)

	static_assert(sizeof(__font_binary_header__) == 36, "Header must be 36 bytes") ;
}