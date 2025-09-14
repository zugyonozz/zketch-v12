#pragma once 

#include "enumerates.hpp"

namespace zketch {

	class logger {
	private :
		static inline HANDLE out_handle() noexcept {
			static HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE) ;
			return h ;
		}

		static inline void set_color(int32_t lv) noexcept {
			WORD attr = 0 ;
			switch (lv) {
				case 0 : attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY ; break ;
				case 1 : attr = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY ; break ;
				case 2 : attr = FOREGROUND_RED | FOREGROUND_INTENSITY ; break ;
				default : break ;
			}
			SetConsoleTextAttribute(out_handle(), attr) ;
		}

		static inline void restore_color(WORD old) noexcept {
			SetConsoleTextAttribute(out_handle(), old) ;
		}

		template <typename T>
		static inline void append_narrow(std::string& out, T v) {
			using U = std::decay_t<T>;

			if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view> || std::is_same_v<U, const char*>) {
				out.append(std::string_view{v}) ;
			} else if constexpr (std::is_same_v<U, char>) {
				out.push_back(v) ;
			} else if constexpr (std::is_integral_v<U>) {
				char buf[32] ;
				auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), v) ;
				out.append(buf, static_cast<size_t>(p - buf)) ;
			} else if constexpr (std::is_floating_point_v<U>) {
				char buf[64] ;
				auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general) ;
				out.append(buf, static_cast<size_t>(p - buf)) ;
			} else if constexpr (std::is_pointer_v<U>) {
				uintptr_t addr = reinterpret_cast<uintptr_t>(v) ; 
				char buf[2 + (sizeof(uintptr_t) * 2) + 1] ;
				buf[0] = '0'; buf[1] = 'x' ;
				auto [p, ec] = std::to_chars(buf + 2, buf + sizeof(buf), addr, 16) ;
				out.append(buf, static_cast<size_t>(p - buf)) ;
			} else {
				static_assert(!sizeof(T*), "Unsupported type for append_narrow; convert to string first") ;
			}
		}

		static inline void widen_utf8_to_wide(const char* src, int src_len, std::wstring& dst) {
			if (src_len <= 0) return ;
			int needed = MultiByteToWideChar(CP_UTF8, 0, src, src_len, nullptr, 0) ;
			if (needed <= 0) return ;
			dst.resize(needed) ;
			MultiByteToWideChar(CP_UTF8, 0, src, src_len, dst.data(), needed) ;
		}

		static inline void widen_utf8_to_wide(const std::string_view& sv, std::wstring& dst) {
			widen_utf8_to_wide(sv.data(), static_cast<int>(sv.size()), dst);
		}

		template <typename T>
		static inline void append_wide(std::wstring& out, T v) {
			using U = std::decay_t<T> ;

			if constexpr (std::is_same_v<U, std::wstring> || std::is_same_v<U, std::wstring_view> || std::is_same_v<U, const wchar_t*>) {
				out.append(std::wstring_view{v}) ;
			} else if constexpr (std::is_same_v<U, wchar_t>) {
				out.push_back(v) ;
			} else if constexpr (std::is_integral_v<U> || std::is_floating_point_v<U>) {
				char buf[64];
				if constexpr (std::is_integral_v<U>) {
					auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), v) ;
					widen_utf8_to_wide(buf, static_cast<int>(p - buf), out) ;
				} else {
					auto [p, ec] = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general) ; 
					widen_utf8_to_wide(buf, static_cast<int>(p - buf), out) ;
				}
			} else if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view> || std::is_same_v<U, const char*>) {
				std::wstring tmp ;
				widen_utf8_to_wide(std::string_view{v}, tmp) ;
				out.append(tmp) ;
			} else if constexpr (std::is_pointer_v<U>) {
				uintptr_t addr = reinterpret_cast<uintptr_t>(v) ;
				char buf[2 + (sizeof(uintptr_t) * 2) + 1] ;
				buf[0] = '0'; buf[1] = 'x' ;
				auto [p, ec] = std::to_chars(buf + 2, buf + sizeof(buf), addr, 16) ;
				std::wstring tmp ;
				widen_utf8_to_wide(buf, static_cast<int>(p - buf), tmp) ;
				out.append(tmp) ;
			} else {
				static_assert(!sizeof(T*), "Unsupported type for append_wide; convert to wstring first") ;
			}
		}

		template <typename ... Args>
		static inline void log_narrow(int32_t lv, const char* tag, Args&& ... args) noexcept {
			std::string buf ;
			buf.reserve(256) ;
			buf.push_back('[') ;
			buf.append(tag) ;
			buf.append("]\t") ;
			(append_narrow(buf, std::forward<Args>(args)), ...) ;
			buf.push_back('\n') ;

			CONSOLE_SCREEN_BUFFER_INFO info ;
			GetConsoleScreenBufferInfo(out_handle(), &info) ;
			WORD old = info.wAttributes ;
			set_color(lv) ;
			DWORD written = 0 ;
			WriteConsoleA(out_handle(), buf.data(), static_cast<DWORD>(buf.size()), &written, nullptr) ;
			restore_color(old) ;
		}

		template <typename ... Args>
		static inline void log_wide(int32_t lv, const wchar_t* tag, Args& ... args) noexcept {
			std::wstring buf ;
			buf.reserve(256) ;
			buf.push_back(L'[') ;
			buf.append(tag) ;
			buf.append(L"]\t") ;
			(append_wide(buf, std::forward<Args>(args)), ...) ;
			buf.push_back(L'\n') ;

			CONSOLE_SCREEN_BUFFER_INFO info ;
			GetConsoleScreenBufferInfo(out_handle(), &info) ;
			WORD old = info.wAttributes ;
			set_color(lv) ;
			DWORD written = 0 ;
			WriteConsoleW(out_handle(), buf.data(), static_cast<DWORD>(buf.size()), &written, nullptr) ;
			restore_color(old) ;
		}

	public :
		template <typename... Args> 
		static inline void info(Args&&... args) noexcept { 
			log_narrow(0, "INFO", std::forward<Args>(args)...) ; 
		}

		template <typename... Args> 
		static inline void warning(Args&&... args) noexcept { 
			log_narrow(1, "WARN", std::forward<Args>(args)...) ; 
		}

		template <typename... Args> 
		static inline void error(Args&&... args) noexcept { 
			log_narrow(2, "ERROR", std::forward<Args>(args)...) ; 
		}

		template <typename... Args> 
		static inline void winfo(Args&&... args) noexcept { 
			log_wide(0, L"INFO", std::forward<Args>(args)...) ; 
		}

		template <typename... Args> 
		static inline void wwarning(Args&&... args) noexcept { 
			log_wide(1, L"WARN", std::forward<Args>(args)...) ; 
		}

		template <typename... Args> 
		static inline void werror(Args&&... args) noexcept { 
			log_wide(2, L"ERROR", std::forward<Args>(args)...) ; 
		}
	} ;

}