#pragma once

#include <memory>
#include <optional>

#include "unit.h"
#include "font.h"

namespace zketch {

	namespace error_handler {
		struct invalid_using_hgdiobj_type {
			const char* what() const noexcept {
				return "invalid use hgdiobj!Error\t: different between assigned data with data use." ;
			}
		} ;
	}

	struct PenStyle {
		enum style__ : uint32_t {
			// ----- Basic Line Styles -----
			Solid        = 0x00000000, // Garis utuh tanpa putus-putus
			Dash         = 0x00000001, // Garis putus-putus panjang
			Dot          = 0x00000002, // Garis titik-titik
			DashDot      = 0x00000003, // Garis kombinasi putus-titik
			DashDotDot   = 0x00000004, // Garis kombinasi putus-titik-titik
			Invisible    = 0x00000005, // Tidak menggambar garis sama sekali
			InsideFrame  = 0x00000006, // Garis di dalam bingkai objek
			UserStyle    = 0x00000007, // Pola garis custom (definisi oleh user)
			Alternate    = 0x00000008, // Garis tipis putus-putus (cosmetic pen saja)

			Mask         = 0x0000000F, // Mask untuk filter style dasar

			// ----- End Cap Styles -----
			RoundedEndCap = 0x00000000, // Ujung garis bulat (default)
			SquareEndCap  = 0x00000100, // Ujung garis persegi
			FlatEndCap    = 0x00000200, // Ujung garis rata/datar
			MaskEndCap    = 0x00000F00, // Mask untuk end cap

			// ----- Join Styles (sambungan antar garis) -----
			JoinRound     = 0x00000000, // Sambungan garis melengkung/bulat
			JoinBevel     = 0x00001000, // Sambungan garis potong miring (bevel)
			JoinMiter     = 0x00002000, // Sambungan garis lancip (miter join)
			JoinMask      = 0x0000F000, // Mask untuk join style

			// ----- Pen Type -----
			Cosmetic      = 0x00000000, // Lebar garis selalu 1 pixel, tidak scaling
			Geometry      = 0x00010000, // Garis mengikuti transformasi dunia (world transform)
			TypeMask      = 0x000F0000, // Mask untuk tipe pen
		} style_ ;

		PenStyle(style__ style) noexcept : style_(style) {}

		operator int32_t() const noexcept {
			return static_cast<int32_t>(style_) ;
		}
	} ;

	enum class enumerate_hgdiobj_ : uint8_t {
		e_HPEN,
		e_HBRUSH,
		e_HFONT,
		e_HBITMAP,
		e_HPALETTE,
		e_HRGN,
		e_HICON,
		e_HENHMETAFILE,
		e_HDC,
		None
	} ;

	template <enumerate_hgdiobj_ flag> 
	struct hgdiobj_flag {
		static constexpr enumerate_hgdiobj_ state = flag ;
	} ;

	template <typename> struct is_selectable_hgdiobj : std::false_type, hgdiobj_flag<enumerate_hgdiobj_::None> {} ;
	template <> struct is_selectable_hgdiobj<HPEN> : std::true_type, hgdiobj_flag<enumerate_hgdiobj_::e_HPEN> {} ;
	template <> struct is_selectable_hgdiobj<HBRUSH> : std::true_type, hgdiobj_flag<enumerate_hgdiobj_::e_HBRUSH> {} ;
	template <> struct is_selectable_hgdiobj<HFONT> : std::true_type, hgdiobj_flag<enumerate_hgdiobj_::e_HFONT> {} ;
	template <> struct is_selectable_hgdiobj<HBITMAP> : std::true_type, hgdiobj_flag<enumerate_hgdiobj_::e_HBITMAP> {} ;
	template <> struct is_selectable_hgdiobj<HRGN> : std::true_type, hgdiobj_flag<enumerate_hgdiobj_::e_HRGN> {} ;

	class selectable_hgdi_ {
	private :
		enum class now_active_ : uint8_t {
			ACTIVE,
			DEFAULT,
		} ;

		static constexpr uint8_t TYPE_MASK  = 0x0F;
    	static constexpr uint8_t STATE_MASK = 0xF0;

		HDC hdc_ ;
		HGDIOBJ active_, default_ ;
		uint8_t data_flag_ ;

		uint8_t generateFlag(enumerate_hgdiobj_ hgdiobj_flag_, now_active_ active_obj_) noexcept {
			return (static_cast<uint8_t>(active_obj_) << 4) | (static_cast<uint8_t>(hgdiobj_flag_) & TYPE_MASK) ;
		}

		enumerate_hgdiobj_ get_hgdiobj_flag(uint8_t flag_data) const noexcept {
			return static_cast<enumerate_hgdiobj_>(flag_data & TYPE_MASK) ;
		}

		now_active_ get_now_active_flag() const noexcept {
			return static_cast<now_active_>((data_flag_ >> 4) & STATE_MASK) ;
		}

	public :
		selectable_hgdi_(const selectable_hgdi_&) = delete ;
		selectable_hgdi_& operator=(const selectable_hgdi_&) = delete ;
		
		template <typename T, typename = std::enable_if_t<is_selectable_hgdiobj<T>::value>> 
		selectable_hgdi_(HDC hdc, T hgdiobj) noexcept : hdc_(hdc), active_(static_cast<HGDIOBJ>(hgdiobj)), data_flag_(generateFlag(is_selectable_hgdiobj<T>::state, now_active_::ACTIVE)) {
			if (hdc_ && active_) {
				HGDIOBJ tmp_ = SelectObject(hdc_, active_) ;
				if (!tmp_) {
					logger::warning("SelectObject returned null.") ;
				} else {
					default_ = tmp_ ;
				}
			} else {
				logger::warning("invalid hdc or active object.") ;
			}
		}

		selectable_hgdi_(selectable_hgdi_&& o) noexcept {
			hdc_ = std::move(o.hdc_) ; 
			active_ = std::move(o.active_) ; 
			default_ = std::move(o.default_) ; 
			data_flag_ = std::exchange(o.data_flag_,0) ;
			o.hdc_ = nullptr ;
			o.active_ = nullptr ;
			o.default_ = nullptr ;
		}

		selectable_hgdi_& operator=(selectable_hgdi_&& o) noexcept {
			if (this != &o) {
				if (hdc_ && default_) 
					SelectObject(hdc_, default_) ;
            	if (active_) 
					DeleteObject(active_) ;
				hdc_ = std::move(o.hdc_) ; 
				active_ = std::move(o.active_) ; 
				default_ = std::move(o.default_);  
				data_flag_ = std::exchange(o.data_flag_,0) ;
				o.hdc_ = nullptr ;
				o.active_ = nullptr ;
				o.default_ = nullptr ;
			}
			return *this ;
		}

		~selectable_hgdi_() noexcept {
			if (hdc_) {
				if (default_)
					SelectObject(hdc_, default_) ;
				if (active_) 
					DeleteObject(active_) ;
			}
		}

		template <typename T, typename = std::enable_if_t<is_selectable_hgdiobj<T>::value>> 
		T UseActive() {
			if (is_selectable_hgdiobj<T>::state != get_hgdiobj_flag(data_flag_)) 
				throw error_handler::invalid_using_hgdiobj_type() ;
			if (get_now_active_flag() == now_active_::ACTIVE) 
				return reinterpret_cast<T>(active_) ;
			if (hdc_) {
				HGDIOBJ prev_ = SelectObject(hdc_, active_);
				if (!prev_) 
					logger::warning("SelectObject(active) returned null") ;
				default_ = prev_ ;
				data_flag_ = generateFlag(get_hgdiobj_flag(data_flag_), now_active_::ACTIVE) ;
			}
			return reinterpret_cast<T>(active_);
		}

		template <typename T, typename = std::enable_if_t<is_selectable_hgdiobj<T>::value>> 
		T UseDefault() {
			if (is_selectable_hgdiobj<T>::state != get_hgdiobj_flag(data_flag_)) 
				throw error_handler::invalid_using_hgdiobj_type() ;
			if (get_now_active_flag() == now_active_::DEFAULT) 
				return reinterpret_cast<T>(default_) ;
			if (hdc_) {
				if (default_) {
					HGDIOBJ prev = SelectObject(hdc_, default_) ;
					if (!prev) 
						logger::warning("SelectObject(default) returned null") ;
				} else {
					logger::warning("default_ is null; cannot SelectObject(default)") ;
				}
				data_flag_ = generateFlag(get_hgdiobj_flag(data_flag_), now_active_::DEFAULT) ;
			}
			return reinterpret_cast<T>(default_);
		}

		HDC GetHDC() const noexcept {
			return hdc_ ;
		}

		template <typename T>
		static selectable_hgdi_ makeObject(HDC hdc, T hgdiobj) noexcept {
			return {hdc, hgdiobj} ;
		}
	} ;

	class Texture {
		friend class Renderer ;
	private :
		HDC hdc_ = nullptr ;
		HBITMAP hbmp_ = nullptr ;
		HBITMAP old_hbmp_ = nullptr ;

	public :
		Texture() noexcept = default ;

		Texture(HWND hwnd, int width, int height) noexcept {
			HDC tmp_hdc_ = GetDC(hwnd) ;
			if (!tmp_hdc_) {
				logger::warning("getDC failed.") ;
				return ; 
			}

			hdc_ = CreateCompatibleDC(tmp_hdc_) ;
			if (!hdc_) {
				logger::warning("CreateCompatibleDC failed.");
				ReleaseDC(hwnd, tmp_hdc_);
				return;
			}

			hbmp_ = CreateCompatibleBitmap(tmp_hdc_, width, height) ;
			if (!hbmp_) {
				logger::warning("CreateCompatibleBitmap failed.") ;
				DeleteDC(hdc_) ;
				hdc_ = nullptr ;
				ReleaseDC(hwnd, tmp_hdc_) ;
				return ; 
			}

			old_hbmp_ = static_cast<HBITMAP>(SelectObject(hdc_, hbmp_)) ;
			logger::info("Successfully creating Texture.") ;
			ReleaseDC(hwnd, tmp_hdc_) ;
		}

		~Texture() noexcept {
			Clear() ;
			logger::info("Texture Destroyed.") ;
		}

		void Clear() noexcept {
			if (hdc_) {
				if (old_hbmp_) 
					SelectObject(hdc_, old_hbmp_) ;
				DeleteDC(hdc_) ;
				hdc_ = nullptr ;
				old_hbmp_ = nullptr ;
			}
			if (hbmp_) {
				DeleteObject(hbmp_) ;
				hbmp_ = nullptr ;
			}
			logger::info("Cleared Texture.") ;
		}

		static std::unique_ptr<Texture> CreateTexture(HWND hwnd, int width, int height) noexcept {
			return std::make_unique<Texture>(hwnd, width, height) ;
		}

		HDC getHDC() const noexcept { 
			return hdc_ ; 
		}

		HBITMAP getBitmap() const noexcept { 
			return hbmp_ ; 
		}

		HDC getHDC() noexcept { 
			return hdc_ ; 
		}

		HBITMAP getBitmap() noexcept { 
			return hbmp_ ; 
		}

		Point getSize() const noexcept {
			BITMAP tmp_{} ;
			if(GetObject(hbmp_, sizeof(tmp_), &tmp_) > 0) 
				return {tmp_.bmWidth, tmp_.bmHeight} ;
			logger::warning("Failed to get Size of texture.") ;
			return {} ;
		}

		bool Empty() const noexcept {
			return (!hdc_ || !hbmp_) ;
		}
	} ;

	class Renderer {
	private :
		static bool isValid(const Texture& texture_) noexcept {
			if (texture_.Empty()) {
				logger::warning("Texture is empty.") ;
				return false ;
			}
			return true ;
		}

	public :
		static void Clear(Texture& texture_, const Color& color = rgba(255, 255, 255, 1.0f)) noexcept {
			if (!isValid(texture_)) return ;
			tagRECT rc_ = {0, 0, texture_.getSize().x, texture_.getSize().y} ;
			selectable_hgdi_ brush_ = selectable_hgdi_::makeObject(texture_.hdc_, CreateSolidBrush(color)) ;
			FillRect(texture_.hdc_, &rc_, brush_.UseActive<HBRUSH>()) ;
		}

		static void DrawRectangle(Texture& texture_, const Color& color_, uint32_t thickness_, const std::optional<Rect>& srcrect_, PenStyle style_ = PenStyle::Solid) noexcept {
			if (!isValid(texture_)) return ;
			selectable_hgdi_ pen_ = selectable_hgdi_::makeObject<HPEN>(texture_.hdc_, CreatePen(style_, thickness_, color_)) ;
			if (!srcrect_) {
				Point size_ = texture_.getSize() ;
				Rectangle(texture_.hdc_, 0, 0, size_.x, size_.y) ;
			} else {
				tagRECT rect_ = srcrect_.value() ;
				Rectangle(texture_.hdc_, rect_.left, rect_.top, rect_.right, rect_.bottom) ;
			}
		}

		static void FillRectangle(Texture& texture_, const Color& color_, const std::optional<Rect>& srcrect_) noexcept {
			if (!isValid(texture_)) return ;
			selectable_hgdi_ brush_ = selectable_hgdi_::makeObject(texture_.hdc_, CreateSolidBrush(color_)) ;
			if (!srcrect_) {
				Rect rect_ = {Point{0, 0}, texture_.getSize()} ;
				tagRECT rect__ = rect_ ;
				FillRect(texture_.hdc_, &rect__, brush_.UseActive<HBRUSH>()) ;
			} else {
				tagRECT rect_ = srcrect_.value() ;
				tagRECT rect__ = rect_ ;
				FillRect(texture_.hdc_, &rect__, brush_.UseActive<HBRUSH>()) ;
			}
		}

		static bool Present(HWND hwnd_, Texture& texture_, const Point& dstpos_, const std::optional<Rect>& srcrect_ = std::nullopt) noexcept {
			if (!isValid(texture_)) return false ;
			HDC window_hdc_ = GetDC(hwnd_) ;
			if (!window_hdc_) {
				logger::warning("getDC failed.") ;
				return false ;
			}

			if (!srcrect_) {
				Point size_ = texture_.getSize() ;
				BitBlt(window_hdc_, dstpos_.x, dstpos_.y, size_.x,  size_.y, texture_.hdc_, 0, 0, SRCCOPY) ;
			} else {
				BitBlt(window_hdc_, dstpos_.x, dstpos_.y, srcrect_->w,  srcrect_->h, texture_.hdc_, srcrect_->x, srcrect_->y, SRCCOPY) ;
			}

			ReleaseDC(hwnd_, window_hdc_) ;
			logger::info("Successfully present texture.") ;
			return true ;
		}
	} ;
}