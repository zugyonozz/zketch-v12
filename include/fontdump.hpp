#pragma once 
#include "fontfield.hpp"

namespace ___FONT_DUMP___ {

	class __font_dump__ {
		friend class zketch::Font ;
		friend class zketch::Application ;

		struct __collect_ctx__ {
			std::set<std::wstring> families ;
		} ;

	private :
		std::string path_csv_ = "installed_fonts.csv" ;
		std::string path_bin_ = "installed_fonts.bin" ;
		bool write_csv_ = true ;
		bool write_bin_ = true ;
		bool write_bom_ = true ;
		bool write_header_ = true ;
		size_t reserve_bytes_ = 1 << 20 ;
		uint32_t charset_ = DEFAULT_CHARSET ;

		static inline void ___trim___(std::string& s) noexcept {
			auto it = s.begin() ;
			while (it != s.end() && std::isspace(static_cast<unsigned char>(*it))) {
				++it ;
			}
			s.erase(s.begin(), it) ;
			
			// Trim right
			auto rit = s.rbegin() ;
			while (rit != s.rend() && std::isspace(static_cast<unsigned char>(*rit))) {
				++rit ;
			}
			s.erase(rit.base(), s.end()) ;
		}

		static inline void ___append_csv_field___(std::string& out, const std::string_view s) {
			const char* p = s.data() ;
			const char* end = p + s.size() ;
			bool needQuote = false ;
			
			// Check if quoting needed
			for ( ; p != end ; ++p) {
				const char c = *p ;
				if (c == '"' || c == ',' || c == '\n' || c == '\r') {
					needQuote = true ;
					break ;
				}
			}
			
			if (!needQuote) {
				out.append(s.data(), s.size()) ;
				return ;
			}
			
			// Escape and quote
			out.push_back('"') ;
			for (const char c : s) {
				if (c == '"') {
					out.append("\"\"") ;
				} else {
					out.push_back(c) ;
				}
			}
			out.push_back('"') ;
		}

		static int CALLBACK CollectFontFamiliesProc(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam) {
			if (!lpelfe || !lParam) return 1 ;
			auto* ctx = reinterpret_cast<__collect_ctx__*>(lParam) ;
			ctx->families.insert(lpelfe->lfFaceName) ;
			return 1 ;
		}

		static inline void ___get_char_widths___(HDC hdc, ___FONT_DATA___::__font_data__& entry) noexcept {
			int32_t widths[___FONT_DATA___::___CHAR_COUNT___] ;
			
			if (GetCharWidth32W(hdc, ___FONT_DATA___::___CHAR_START___, ___FONT_DATA___::___CHAR_END___, widths)) {
				for (int i = 0 ; i < ___FONT_DATA___::___CHAR_COUNT___ ; ++i) {
					entry.char_widths_[i] = static_cast<float>(widths[i]) ;
				}
			} else {
				for (int i = 0 ; i < ___FONT_DATA___::___CHAR_COUNT___ ; ++i) {
					entry.char_widths_[i] = 0.0f ;
				}
			}
		}

		[[nodiscard]] static inline const ___FONT_DATA___::__font_data__* ___find_font___(const std::unordered_map<std::string, ___FONT_DATA___::__font_data__>& fontMap, const std::string_view name, uint8_t style = 0) noexcept {
			std::string key ;
			key.reserve(name.size() + 2) ;
			key.append(name) ;
			key.push_back('|') ;
			key.push_back('0' + style) ;
			
			const auto it = fontMap.find(key) ;
			return (it != fontMap.end()) ? &it->second : nullptr ;
		}

		[[nodiscard]] static inline std::optional<___FONT_DATA___::__font_data__> ___try_create_font___(HDC hdc, const std::wstring& fontname, int reqWeight, bool reqItalic) noexcept {
			LOGFONTW lf {} ;
			wcscpy_s(lf.lfFaceName, fontname.c_str()) ;
			lf.lfHeight = -16 ;
			lf.lfWeight = reqWeight ;
			lf.lfItalic = reqItalic ? TRUE : FALSE ;
			lf.lfCharSet = DEFAULT_CHARSET ;
			lf.lfQuality = CLEARTYPE_QUALITY ;
			
			HFONT hFont = CreateFontIndirectW(&lf) ;
			if (!hFont) {
				return std::nullopt ;
			}
			
			HGDIOBJ oldFont = SelectObject(hdc, hFont) ;
			
			wchar_t actualName[256] ;
			const int nameLen = GetTextFaceW(hdc, 256, actualName) ;
			if (nameLen <= 0) {
				SelectObject(hdc, oldFont) ;
				DeleteObject(hFont) ;
				return std::nullopt ;
			}
			
			TEXTMETRICW tm {} ;
			if (!GetTextMetricsW(hdc, &tm)) {
				SelectObject(hdc, oldFont) ;
				DeleteObject(hFont) ;
				return std::nullopt ;
			}
			
			char nameBuffer[512] ;
			const int utf8Len = WideCharToMultiByte(
				CP_UTF8, 0, actualName, nameLen, 
				nameBuffer, sizeof(nameBuffer), nullptr, nullptr) ;
			
			if (utf8Len <= 0) {
				SelectObject(hdc, oldFont) ;
				DeleteObject(hFont) ;
				return std::nullopt ;
			}
			
			std::string name(nameBuffer, utf8Len) ;
			___trim___(name) ;
			

			___FONT_DATA___::__font_data__ entry {} ;


			entry.SetFontName(name) ;
			entry.height_ = static_cast<float>(tm.tmHeight) ;
			entry.ascent_ = static_cast<float>(tm.tmAscent) ;
			entry.descent_ = static_cast<float>(tm.tmDescent) ;
			entry.weight_ = static_cast<float>(tm.tmWeight) ;
			
			const bool isBold = (tm.tmWeight >= FW_BOLD) ;
			const bool isItalic = (tm.tmItalic != 0) ;
			entry.style_ = (isBold ? 1 : 0) | (isItalic ? 2 : 0) ;
			___get_char_widths___(hdc, entry) ;
			SelectObject(hdc, oldFont) ;
			DeleteObject(hFont) ;
			
			return entry ;
		}

		[[nodiscard]] static inline std::optional<std::pair<size_t, size_t>> ___dump_installed_fonts___(const __font_dump__& opt) noexcept {
			HDC screen = GetDC(nullptr) ;
			if (!screen) {
				return std::nullopt ;
			}
			
			HDC dc = CreateCompatibleDC(screen) ;
			ReleaseDC(nullptr, screen) ;
			if (!dc) {
				return std::nullopt ;
			}

			#ifdef FONT_DEBUG
				logger::info("___dump_installed_fonts___ - Collecting font families...") ;
			#endif

			__collect_ctx__ ctxs ;
			LOGFONTW lf {} ;
			lf.lfCharSet = static_cast<BYTE>(opt.charset_) ;
			EnumFontFamiliesExW(dc, &lf, CollectFontFamiliesProc, reinterpret_cast<LPARAM>(&ctxs), 0) ;
			
			const size_t familyCount = ctxs.families.size() ;

			#ifdef FONT_DEBUG
				logger::info("___dump_installed_fonts___ - Found font families", familyCount) ;
			#endif

			std::vector<___FONT_DATA___::__font_data__> entries ;
			entries.reserve(familyCount * 4) ;

			std::string csvOutput ;
			if (opt.write_csv_) {
				csvOutput.reserve(opt.reserve_bytes_) ;
				if (opt.write_bom_) {
					csvOutput.append("\xEF\xBB\xBF") ;
				}
				if (opt.write_header_) {
					csvOutput.append("Name,Height,Ascent,Descent,Weight,Style") ;
					for (int i = ___FONT_DATA___::___CHAR_START___ ; i <= ___FONT_DATA___::___CHAR_END___ ; ++i) {
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(i)) ;
					}
					csvOutput.push_back('\n') ;
				}
			}
			
			std::unordered_set<std::string> seen ;
			seen.reserve(familyCount * 4) ;
			
			constexpr std::pair<int, bool> styles[4] = {
				{FW_NORMAL, false},
				{FW_BOLD, false},
				{FW_NORMAL, true},
				{FW_BOLD, true}
			} ;
			
			for (const auto& family : ctxs.families) {
				for (const auto& [weight, italic] : styles) {
					auto entryOpt = ___try_create_font___(dc, family, weight, italic) ;
					if (!entryOpt) {
						continue ;
					}

					___FONT_DATA___::__font_data__ entry = *entryOpt ;

					const std::string_view name_view(entry.fontname_.data(), strlen(entry.fontname_.data())) ;
					std::string key ;
					key.reserve(name_view.size() + 2) ;
					key.append(name_view) ;
					key.push_back('|') ;
					key.push_back('0' + entry.style_) ;
					
					if (!seen.insert(key).second) {
						continue ;
					}
					
					entries.push_back(entry) ;
					
					if (opt.write_csv_) {
						___append_csv_field___(csvOutput, name_view) ;
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.height_)) ;
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.ascent_)) ;
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.descent_)) ;
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.weight_)) ;
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.style_)) ;
						
						// Write char widths
						for (size_t i = 0 ; i < entry.char_widths_.size() ; ++i) {
							csvOutput.push_back(',') ;
							csvOutput.append(std::to_string(entry.char_widths_[i])) ;
						}
						csvOutput.push_back('\n') ;
					}
				}
			}

			DeleteDC(dc) ;

			size_t csvBytes = 0 ;
			if (opt.write_csv_ && !csvOutput.empty()) {
				std::ofstream csvFile(opt.path_csv_, std::ios::binary) ;
				if (csvFile) {
					csvFile.write(csvOutput.data(), static_cast<std::streamsize>(csvOutput.size())) ;
					csvBytes = csvOutput.size() ;

					#ifdef FONT_DEBUG
						logger::info("___dump_installed_fonts___ - CSV saved", opt.out_path_csv) ;
					#endif
				}
			}

			size_t binBytes = 0 ;
			if (opt.write_bin_ && !entries.empty()) {
				std::ofstream binFile(opt.path_bin_, std::ios::binary) ;
				if (binFile) {

					___FONT_DATA___::__font_binary_header__ header ;

					header.count_ = static_cast<uint32_t>(entries.size()) ;
					header.timestamp_ = static_cast<uint64_t>(time(nullptr)) ;
					
					binFile.write(reinterpret_cast<const char*>(&header), sizeof(header)) ;
					binFile.write(reinterpret_cast<const char*>(entries.data()), static_cast<std::streamsize>(entries.size() * sizeof(___FONT_DATA___::__font_data__))) ;
					
					binBytes = sizeof(header) + entries.size() * sizeof(___FONT_DATA___::__font_data__) ;

					#ifdef FONT_DEBUG
						logger::info("___dump_installed_fonts___ - Binary saved", opt.out_path_bin) ;
						logger::info("___dump_installed_fonts___ - Entry size", sizeof(Font)) ;
					#endif
				}
			}

			return std::make_pair(entries.size(), std::max(csvBytes, binBytes)) ;
		}

	public :
		__font_dump__() {
			if (!___ACCESSOR___::____accessor____::___is_granted___()) {
				throw ___ERROR_HANDLER___::no_have_access() ;
			} 
		}

		static bool CreateDumpFonts(const char* filename_without_ext, const char* path, uint32_t reserve_bytes = 1 << 20, bool write_bin = true, bool write_csv = false) noexcept {
			___ACCESSOR___::____accessor____::___open_access___() ;

			__font_dump__ opt ;
			opt.path_csv_ = std::string(filename_without_ext) + path + ".csv" ;
			opt.path_bin_ = std::string(filename_without_ext) + path + ".bin" ;
			opt.write_bin_ = write_bin ;
			opt.write_csv_ = write_csv ;
			opt.reserve_bytes_ = reserve_bytes ;

			const auto res = ___dump_installed_fonts___(opt) ;

			___ACCESSOR___::____accessor____::___close_access___() ;
			
			return res.has_value() ;
		}

		[[nodiscard]] static inline std::optional<std::unordered_map<std::string, ___FONT_DATA___::__font_data__>> LoadFontsFromBin(const std::string& filename) noexcept {
			std::ifstream file(filename, std::ios::binary) ;
			if (!file) {
				return std::nullopt ;
			}
			
			file.seekg(0, std::ios::end) ;
			const auto fileSize = file.tellg() ;
			file.seekg(0, std::ios::beg) ;
			
			if (fileSize < static_cast<std::streamoff>(sizeof(___FONT_DATA___::__font_binary_header__))) {
				return std::nullopt ;
			}

			___ACCESSOR___::____accessor____::___open_access___() ;

			___FONT_DATA___::__font_binary_header__ header ;

			file.read(reinterpret_cast<char*>(&header), sizeof(header)) ;
			
			if (!file || !header.IsValid()) {

				___ACCESSOR___::____accessor____::___close_access___() ;

				return std::nullopt ;
			}
			
			const size_t expectedSize = sizeof(___FONT_DATA___::__font_binary_header__) + header.count_ * sizeof(___FONT_DATA___::__font_data__) ;
			if (static_cast<size_t>(fileSize) < expectedSize) {

				___ACCESSOR___::____accessor____::___close_access___() ;

				return std::nullopt ;
			}
			
			std::unordered_map<std::string, ___FONT_DATA___::__font_data__> fontMap ;
			fontMap.reserve(header.count_) ;
			
			std::vector<___FONT_DATA___::__font_data__> entries(header.count_) ;
			file.read(reinterpret_cast<char*>(entries.data()), static_cast<std::streamsize>(header.count_ * sizeof(___FONT_DATA___::__font_data__))) ;
			
			if (!file) {

				___ACCESSOR___::____accessor____::___close_access___() ;

				return std::nullopt ;
			}
			
			for (auto& entry : entries) {
				const std::string_view name_view(entry.fontname_.data(), strlen(entry.fontname_.data())) ;
				std::string key ;
				key.reserve(name_view.size() + 2) ;
				key.append(name_view) ;
				key.push_back('|') ;
				key.push_back('0' + static_cast<char>(entry.style_)) ;
				fontMap.emplace(std::move(key), std::move(entry)) ;
			}

			___ACCESSOR___::____accessor____::___close_access___() ;

			return fontMap ;
		}
	} ;
}